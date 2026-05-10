#pragma once

#include "common/value.h"
#include "sql/operator/physical_operator.h"

class FieldMeta;
class Table;
class Trx;

class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table, const FieldMeta *field_meta, Value value);
  virtual ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }
  OpType               get_op_type() const override { return OpType::UPDATE; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  RC make_new_record(const Record &old_record, Record &new_record);

private:
  Table           *table_      = nullptr;
  const FieldMeta *field_meta_ = nullptr;
  Value            value_;
  Trx             *trx_        = nullptr;
  vector<Record>   old_records_;
  vector<Record>   new_records_;
};
