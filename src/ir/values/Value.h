
#include "../../ValueVisitor/Visitor.h"
#include "../../ast/types/Type.h"
#include "../../common.h"
#include "../ModuleHolder.h"
#include "../id.h"

#ifndef __SNOWBALL_VALUE_H_
#define __SNOWBALL_VALUE_H_

#define SN_GENERATOR_VISITS \
  void visit(codegen::ValueVisitor* v) override { v->visit(this); };

/**
 * @brief Snowball values
 */
namespace snowball {
namespace ir {

/// In snowball's internal IR,
class Value : public ModuleHolder, public DBGObject {
  types::Type* type;

  Value(const Value*&) = delete;
  Value(const Value&) = delete;
  Value(const Value&&) = delete;
  Value& operator=(const Value&) = delete;

 public:
  virtual ~Value() noexcept = default;
  Value() = default;
  Value(types::Type* p_type) { type = p_type; };

  /// @return get value's type
  virtual types::Type* getType() const { return type; }
  /// @brief get value's type
  virtual void setType(types::Type* p_type) { type = p_type; }

  // Generate helper function
  virtual void visit(codegen::ValueVisitor* v) = 0;
};

} // namespace ir
} // namespace snowball

#endif // __SNOWBALL_VALUE_H_
