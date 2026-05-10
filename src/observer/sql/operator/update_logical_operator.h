#pragma once

#include "common/value.h"
#include "sql/operator/logical_operator.h"

class Table;
class FieldMeta;

class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table, const FieldMeta *field_meta, Value value);
  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }
  OpType              get_op_type() const override { return OpType::LOGICALUPDATE; }

  Table           *table() const { return table_; }
  const FieldMeta *field_meta() const { return field_meta_; }
  const Value     &value() const { return value_; }

private:
  Table           *table_      = nullptr;
  const FieldMeta *field_meta_ = nullptr;
  Value            value_;
};
