#include "../../../../ir/values/Call.h"
#include "../../../../services/OperatorService.h"
#include "../../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Expression::NewInstance) {
  auto call = p_node->getCall();
  auto expr = p_node->getType();

  assert(utils::cast<Expression::TypeRef>(expr));
  auto ident = Syntax::N<Expression::Identifier>(
          services::OperatorService::getOperatorMangle(services::OperatorService::CONSTRUCTOR)
  );
  ident->setDBGInfo(expr->getDBGInfo());
  auto index = Syntax::N<Expression::Index>(expr, ident, true);
  index->setDBGInfo(expr->getDBGInfo());

  call->setCallee(index);
  auto c = utils::dyn_cast<ir::Call>(trans(call));
  assert(c != nullptr);

  auto typeRef = utils::cast<Expression::TypeRef>(expr);
  auto type = transformSizedType(typeRef, false, "Can't create instance of unsized type '%s'!");

  // Make a copy of the value
  auto v = getBuilder().createObjectInitialization(p_node->getDBGInfo(), type, c->getCallee(), c->getArguments());
  this->value = v;
}

} // namespace Syntax
} // namespace snowball