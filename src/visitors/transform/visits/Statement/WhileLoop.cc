#include "../../../../ir/values/WhileLoop.h"

#include "../../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Statement::WhileLoop) {
  auto cond = trans(p_node->getCondition());
  auto expr = getBooleanValue(cond);
  auto block = trans(p_node->getBlock());
  auto body = utils::dyn_cast<ir::Block>(block);
  std::shared_ptr<ir::WhileLoop> loop;
  if (auto x = p_node->getForCond()) {
    loop = getBuilder().createFromForLoop(p_node->getDBGInfo(), expr, body, trans(x));
  } else {
    loop = getBuilder().createWhileLoop(p_node->getDBGInfo(), expr, body, p_node->isDoWhile());
  }
  this->value = utils::dyn_cast<ir::Value>(loop);
}

} // namespace Syntax
} // namespace snowball