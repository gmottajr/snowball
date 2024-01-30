
#include "../../Transformer.h"

#include <optional>

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

Transformer::StoreType Transformer::getFromIdentifier(Expression::Identifier* s) {
  auto g = utils::cast<Expression::GenericIdentifier>(s);
  auto generics = (g != nullptr) ? g->getGenerics() : std::vector<Expression::TypeRef*> {};
  return getFromIdentifier(s->getDBGInfo(), s->getIdentifier(), generics);
}
Transformer::StoreType Transformer::getFromIdentifier(
  DBGSourceInfo* dbgInfo,
  const std::string identifier,
  std::vector<Expression::TypeRef*> generics,
  const std::string p_uuid
) {
  // Transform the base first
  auto[item, success] = ctx->getItem(identifier);
  if (success && p_uuid.empty()) {
    if (item->isVal()) {
      // TODO: it should not be getValue, it should have it's own
      // value
      auto val = item->getValue();
      return {val, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
    } else if (item->isFunc()) {
      return {std::nullopt, std::nullopt, item->getFunctions(), std::nullopt, std::nullopt};
    } else if (item->isType()) {
      return {std::nullopt, item->getType(), std::nullopt, std::nullopt, std::nullopt};
    } else if (item->isType()) {
      return {std::nullopt, item->getType(), std::nullopt, std::nullopt, std::nullopt};
    } else if (item->isModule()) {
      return {std::nullopt, std::nullopt, std::nullopt, std::nullopt, item->getModule()};
    } else if (item->isMacro()) {
      E<SYNTAX_ERROR>(
        dbgInfo,
      "Macros cannot be used as values!", {
        .info = "This is the macro that was used",
        .note = "Macros are not values, they are used to generate code at compile time.",
        .help = "If you want to use a macro as a value, you can use the '@' symbol before \nand "
        "identifier to use a function-like macro."
      }
      );
    } else if (item->isAlias()) {
      if (auto x = utils::cast<Expression::Index>(item->getASTAlias())) {
        if (generics.size() > 0) {
          auto clone = N<Expression::Index>(*x);
          clone->unsafeSetidentifier(N<Expression::GenericIdentifier>(clone->getIdentifier()->getIdentifier(), generics));
          x = clone;
        }
        auto[r, _] = getFromIndex(dbgInfo, x, x->isStatic);
        auto[val, ty, funcs, overloads, mod, _canPrivate] = r;
        return {val, ty, funcs, overloads, mod};
      } else if (auto x = utils::cast<Expression::Identifier>(item->getASTAlias())) {
        return getFromIdentifier(dbgInfo, x->getIdentifier(), generics, p_uuid);
      } else {
        assert(false && "BUG: Unhandled alias type!");
      }
    } else {
      assert(false && "BUG: Unhandled value type!");
    }
  }
  int uuidStackIndex = -1;
  std::string uuidStackOverride = "";
fetchFromUUID:
  // If we can't find it in the stack, we need to fetch it from the cache
  auto uuid = uuidStackOverride.empty() ?
              p_uuid.empty() ? ctx->createIdentifierName(identifier, false) : (p_uuid + "." + identifier) :
              (uuidStackOverride + "." + identifier);
  std::optional<std::deque<std::shared_ptr<ir::Func>>> funcs = std::nullopt;
  if (auto x = ctx->cache->getTransformedFunction(uuid)) {
    assert(x->get()->isFunc());
    funcs = x->get()->getFunctions();
  }
  if (auto x = ctx->cache->getTransformedType(uuid)) {
    auto ty = new Expression::TypeRef(identifier, dbgInfo, generics);
    for (auto t : x.value()) {
      assert(t->isType());
      if (typeGenericsMatch(ty, t->getType())) {
        return {std::nullopt, t->getType(), std::nullopt, std::nullopt, std::nullopt};
      }
    }
  }
  if (auto t = ctx->cache->getType(uuid)) {
    auto ty = new Expression::TypeRef(identifier, dbgInfo, generics);
    return {std::nullopt, transformTypeFromBase(uuid, t.value(), ty), std::nullopt, std::nullopt, std::nullopt};
  }
  std::optional<std::deque<Cache::FunctionStore>> overloads = std::nullopt;
  if (auto x = ctx->cache->getFunction(uuid)) { overloads = x; }
  if (funcs || overloads) { return {std::nullopt, std::nullopt, funcs, overloads, std::nullopt}; }
  if (auto mod = ctx->cache->getModule(uuid)) { return {std::nullopt, std::nullopt, std::nullopt, std::nullopt, mod}; }
  if ((size_t)(uuidStackIndex + 1) < ctx->uuidStack.size()) {
    uuidStackIndex++;
    uuidStackOverride = ctx->uuidStack[uuidStackIndex];
    goto fetchFromUUID;
  }
  auto ty = new Expression::TypeRef(identifier, dbgInfo, generics);
  if (auto special = transformSpecialType(ty)) {
    return {std::nullopt, special, std::nullopt, std::nullopt, std::nullopt};
  }
  return {std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
}

} // namespace Syntax
} // namespace snowball