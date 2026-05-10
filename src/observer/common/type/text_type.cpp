#include "common/type/text_type.h"

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/value.h"

int TextType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::TEXTS || left.attr_type() == AttrType::CHARS, "left type is not string");
  ASSERT(right.attr_type() == AttrType::TEXTS || right.attr_type() == AttrType::CHARS, "right type is not string");
  return common::compare_string(
      (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
}

RC TextType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::CHARS: {
      result.set_string(val.value_.pointer_value_, val.length_);
      return RC::SUCCESS;
    }
    case AttrType::TEXTS: {
      result.set_string(val.value_.pointer_value_, val.length_);
      result.set_type(AttrType::TEXTS);
      return RC::SUCCESS;
    }
    default: {
      LOG_WARN("unsupported cast from text to type %d", type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }
}

RC TextType::set_value_from_str(Value &val, const string &data) const
{
  val.set_string(data.c_str(), data.length());
  val.set_type(AttrType::TEXTS);
  return RC::SUCCESS;
}

int TextType::cast_cost(AttrType type)
{
  if (type == AttrType::TEXTS) {
    return 0;
  }
  if (type == AttrType::CHARS) {
    return 1;
  }
  return INT32_MAX;
}

RC TextType::to_string(const Value &val, string &result) const
{
  if (val.value_.pointer_value_ == nullptr) {
    result.clear();
  } else {
    result.assign(val.value_.pointer_value_, val.length_);
  }
  return RC::SUCCESS;
}