#include "../../../../ir/values/Cast.h"
#include "../../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Statement::Namespace) {
  auto name = p_node->getName();
  auto body = p_node->getBody();
  auto uuid = ctx->createIdentifierName(name);
  if (!ctx->generateFunction) {
    if (ctx->getInScope(name, ctx->currentScope()).second)
      E<VARIABLE_ERROR>(p_node, FMT("Namespace '%s' is already defined in the current scope!", name.c_str()));
    auto mod = std::make_shared<ir::Module>(getNameWithBase(name), uuid);
    mod->setSourceInfo(ctx->module->getSourceInfo());
    ctx->uuidStack.push_back(ctx->module->getUniqueName());
    auto sharedModule = std::make_shared<Item>(mod);
    auto backup = ctx->module;
    ctx->module = mod;
    ctx->withScope([&]() { // ERROR: defined macros inside namespaces get's deleted since the scope also gets deleted
                   for (auto x : body) { SN_TRANSFORMER_CAN_GENERATE(x) trans(x); }
                   });
    ctx->module = backup;
    addModule(mod);
    ctx->addItem(name, sharedModule);
    ctx->uuidStack.pop_back();
  } else {
    auto item = ctx->cache->getModule(uuid);
    assert(item);
    auto mod = item.value();
    ctx->uuidStack.push_back(ctx->module->getUniqueName());
    auto backup = ctx->module;
    ctx->module = mod;
    ctx->withScope([&]() {
                   for (auto x : body) { trans(x); }
                   });
    ctx->module = backup;
    ctx->uuidStack.pop_back();
  }
}

} // namespace Syntax
} // namespace snowball