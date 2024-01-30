#include "../../../Transformer.h"

#include <optional>

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Expression::LambdaFunction) {
  if (!ctx->generateFunction) return;
  auto node = p_node->getFunc();
  auto parent = ctx->getCurrentFunction();
  assert(parent != nullptr);
  // Get the respective return type for this function
  auto returnType = transformType(node->getRetType());
  // Create a new function value and store it's return type.
  char l[] = _SNOWBALL_LAMBDA_FUNCTIONS;
  auto name = "[" + p_node->getSourceInfo()->getPath() + "@" + std::to_string(p_node->getDBGInfo()->line) + " " + l + "]";
  auto fn = getBuilder().createFunction(node->getDBGInfo(), name, false, node->isVariadic(), true);
  fn->setParent(ctx->getCurrentClass());
  fn->setParentScope(ctx->getCurrentFunction());
  fn->setScopeIndex(ctx->getScopeIndex());
  fn->setRetTy(returnType);
  fn->setPrivacy(Statement::Privacy::PRIVATE);
  fn->setStatic(false);
  fn->addAttribute(Attributes::INTERNAL_LINKAGE);
  fn->setAttributes(node);
  ir::Func::FunctionArgs newArgs = {};
  for (size_t i = 0; i < node->getArgs().size(); i++) {
    auto arg = node->getArgs().at(i);
    auto a = getBuilder().createArgument(node->getDBGInfo(), arg->getName(), fn->isConstructor() + i,
                                         arg->hasDefaultValue() ? arg->getDefaultValue() : nullptr, ctx->getScopeIndex());
    a->setType(transformType(arg->getType()));
    newArgs.insert(newArgs.end(), {arg->getName(), a});
  }
  fn->setArgs(newArgs);
  auto typeIdentifier = N<Expression::GenericIdentifier>("Function", std::vector<Expression::TypeRef*> {types::FunctionType::from(fn.get())->toRef()});
  auto coreIdentifier = N<Expression::Identifier>("std");
  auto typeRefNode = N<Expression::Index>(coreIdentifier, typeIdentifier, true);
  auto typeRef = TR(typeRefNode, "std::Function", p_node->getDBGInfo(), "");
  auto fnType = transformType(typeRef);
  fn->setType(fnType);
  // Generate a bodied for functions that have
  // them defined.
  if (auto bodiedFn = utils::cast<Statement::BodiedFunction>(node)) {
    auto backupFunction = ctx->getCurrentFunction();
    ctx->setCurrentFunction(fn);
    ctx->withScope([&]() {
                     for (auto arg : newArgs) {
                       auto refItem = std::make_shared<transform::Item>(transform::Item::Type::VALUE, arg.second);
                       ctx->addItem(arg.first, refItem);
                     }
                     auto body = bodiedFn->getBody();
                     if (!fn->isConstructor() && !bodyReturns(body->getStmts()) &&
      !((utils::cast<types::NumericType>(returnType)) || (utils::cast<types::VoidType>(returnType)))) {
      E<TYPE_ERROR>(
        node, "Function lacks ending return statement!", {.info = "Function does not return on all paths!"});
      }
    auto functionBody = utils::dyn_cast<ir::Block>(trans(body));
                        fn->setBody(functionBody);
                   });
    ctx->setCurrentFunction(backupFunction);
  } else if (auto e = utils::cast<Statement::LLVMFunction>(node)) {
    fn->setLLVMBody(getLLVMBody(e->getBody(), e->getTypesUsed()));
  }
  ctx->module->addFunction(fn);
  this->value = fn;
}

} // namespace Syntax
} // namespace snowball