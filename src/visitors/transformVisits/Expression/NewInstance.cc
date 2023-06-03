#include "../../../ir/values/Call.h"
#include "../../../services/OperatorService.h"
#include "../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Expression::NewInstance) {
    auto call = p_node->getCall();
    auto expr = call->getCallee();

    assert(utils::cast<Expression::TypeRef>(expr));
    auto ident = Syntax::N<Expression::Identifier>(
        services::OperatorService::getOperatorMangle(
            services::OperatorService::CONSTRUCTOR));
    ident->setDBGInfo(expr->getDBGInfo());
    auto index = Syntax::N<Expression::Index>(expr, ident, true);
    index->setDBGInfo(expr->getDBGInfo());

    call->setCallee(index);
    call->accept(this);

    auto c = utils::dyn_cast<ir::Call>(this->value);
    assert(c != nullptr);

    auto typeRef = utils::cast<Expression::TypeRef>(expr);
    auto type = transformType(typeRef);

    // Make a copy of the value
    auto v = ctx->module->N<ir::ObjectInitialization>(p_node->getDBGInfo(), c->getCallee(),
                                      c->getArguments());
    v->initializeAtHeap = p_node->atHeap();
    v->setType(v->initializeAtHeap ? type->getPointerTo() : type);
    this->value = v;
}

} // namespace Syntax
} // namespace snowball