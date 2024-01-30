
#include "../../ValueVisitor/Visitor.h"
#include "../../ast/syntax/nodes.h"
#include "../../ast/types/Type.h"
#include "../../common.h"
#include "Value.h"

#include <map>
#include <vector>

#ifndef __SNOWBALL_VARIABLE_VALUE_H_
#define __SNOWBALL_VARIABLE_VALUE_H_

namespace snowball {
namespace ir {

/// @brief Representation of a variable access inside the IR.
class Variable : public IdMixin, public AcceptorExtend<Variable, Value> {
  // Identifier we use to fetch the variable
  std::string identifier;
  // If the variable is marked as an argument, we can make certain
  // actions for this specific occasion.
  bool _isArgument = false;
  // Whether or not the variable is mutable
  bool _isMutable = false;
  // Scope index of the variable
  int scopeIndex = -1;
  // Whether or not the variable is used inside a lambda
  bool _isUsedInLambda = false;
  // Function where the variable is defined in
  Func* parentFunc = nullptr;

 public:
  // Create a new variable declaration
  Variable(const std::string& identifier, bool isArgument = false, bool isMutable = false, int scopeIndex = -1)
    : identifier(identifier), _isArgument(isArgument), _isMutable(isMutable), scopeIndex(scopeIndex) {};

  /// @return Variable identifier
  auto getIdentifier() const { return identifier; }
  /// @return Wether or not the variable points to an argument
  auto isArgument() { return _isArgument; }
  /// @return true if the variable is mutable
  auto isMutable() { return _isMutable; }
  /// @return Scope index of the variable
  auto getScopeIndex() { return scopeIndex; }
  /// @return true if the variable is used inside a lambda
  auto isUsedInLambda() { return _isUsedInLambda; }
  /// @brief Set if the variable is used inside a lambda
  void setUsedInLambda(bool used = true) { _isUsedInLambda = used; }
  /// @return Function where the variable is defined in
  auto getParentFunc() { assert(isUsedInLambda()); return parentFunc; }
  /// @brief Set the function where the variable is defined in
  void setParentFunc(Func* func) { assert(isUsedInLambda()); parentFunc = func; }

  /// @brief Set the mutable flag
  void setMutability(bool isMutable) { _isMutable = isMutable; }

  // Set a visit handler for the generators
  SN_GENERATOR_VISITS
};

} // namespace ir
} // namespace snowball

#endif // __SNOWBALL_VARIABLE_VALUE_H_
