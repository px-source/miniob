#include "sql/operator/update_physical_operator.h"
#include <cstdlib>
#include <cstring>
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "storage/field/field_meta.h"
#include "storage/record/text_lob.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

UpdatePhysicalOperator::UpdatePhysicalOperator(Table *table, const FieldMeta *field_meta, Value value)
    : table_(table), field_meta_(field_meta), value_(std::move(value))
{}

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    LOG_WARN("update operator has no child");
    return RC::INTERNAL;
  }

  unique_ptr<PhysicalOperator> &child = children_[0];
  RC                            rc    = child->open(trx);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to open update child operator. rc=%s", strrc(rc));
    return rc;
  }

  trx_ = trx;
  while (OB_SUCC(rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      LOG_WARN("failed to get current tuple from child");
      return RC::INTERNAL;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record   &record    = row_tuple->record();

    Record old_record;
    rc = old_record.copy_data(record.data(), record.len());
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to copy old record data. rc=%s", strrc(rc));
      return rc;
    }
    old_record.set_rid(record.rid());

    Record new_record;
    rc = make_new_record(old_record, new_record);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to make new record. rc=%s", strrc(rc));
      return rc;
    }

    old_records_.emplace_back(std::move(old_record));
    new_records_.emplace_back(std::move(new_record));
  }

  child->close();
  if (rc != RC::RECORD_EOF) {
    LOG_WARN("failed to scan child records for update. rc=%s", strrc(rc));
    return rc;
  }

  rc = RC::SUCCESS;
  for (size_t i = 0; i < old_records_.size() && OB_SUCC(rc); i++) {
    rc = trx_->delete_record(table_, old_records_[i]);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to delete old record while updating. rc=%s", strrc(rc));
      return rc;
    }

    rc = trx_->insert_record(table_, new_records_[i]);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to insert new record while updating. rc=%s", strrc(rc));
      return rc;
    }
  }

  return rc;
}

RC UpdatePhysicalOperator::next()
{
  return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close()
{
  old_records_.clear();
  new_records_.clear();
  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::make_new_record(const Record &old_record, Record &new_record)
{
  const int record_size = table_->table_meta().record_size();
  char     *record_data = (char *)malloc(record_size);
  if (nullptr == record_data) {
    LOG_WARN("failed to allocate record data. size=%d", record_size);
    return RC::NOMEM;
  }

  memcpy(record_data, old_record.data(), record_size);

  const int field_offset = field_meta_->offset();
  const int field_len    = field_meta_->len();
  if (field_meta_->type() == AttrType::TEXTS) {
    if (field_len < static_cast<int>(sizeof(TextLobLocator))) {
      free(record_data);
      LOG_WARN("invalid text field length. field=%s, len=%d", field_meta_->name(), field_len);
      return RC::INTERNAL;
    }
    if (table_->lob_handler() == nullptr) {
      free(record_data);
      LOG_WARN("lob handler not initialized. table=%s", table_->name());
      return RC::INTERNAL;
    }

    int64_t offset = 0;
    RC      rc     = table_->lob_handler()->insert_data(offset, value_.length(), value_.data());
    if (OB_FAIL(rc)) {
      free(record_data);
      LOG_WARN("failed to write text value to lob. rc=%s", strrc(rc));
      return rc;
    }

    TextLobLocator locator;
    locator.offset = offset;
    locator.length = value_.length();
    locator.magic  = TEXT_LOB_MAGIC;

    memset(record_data + field_offset, 0, field_len);
    memcpy(record_data + field_offset, &locator, sizeof(locator));
  } else if (field_meta_->type() == AttrType::CHARS) {
    memset(record_data + field_offset, 0, field_len);

    size_t copy_len = field_len;
    if (copy_len > static_cast<size_t>(value_.length())) {
      copy_len = value_.length() + 1;
    }
    memcpy(record_data + field_offset, value_.data(), copy_len);
  } else {
    memcpy(record_data + field_offset, value_.data(), field_len);
  }

  new_record.set_data_owner(record_data, record_size);
  return RC::SUCCESS;
}
