
#include "../../../ir/values/IndexExtract.h"
#include "../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;
using namespace snowball::services;

namespace snowball {
namespace Syntax {

std::pair<std::tuple<std::optional<std::shared_ptr<ir::Value>>,
                     std::optional<types::Type*>,
                     std::optional<std::deque<std::shared_ptr<ir::Func>>>,
                     std::optional<std::deque<Cache::FunctionStore>>,
                     std::optional<std::shared_ptr<ir::Module>>,
                     bool /* Accept private members */>,
          std::optional<std::shared_ptr<ir::Value>>>
Transformer::getFromIndex(DBGSourceInfo* dbgInfo, Expression::Index* index, bool isStatic) {
  auto getFromType = [&](types::Type* type,
                         std::shared_ptr<ir::Value> value =
                                 nullptr) -> std::tuple<std::optional<std::shared_ptr<ir::Value>>,
                                                        std::optional<types::Type*>,
                                                        std::optional<std::deque<std::shared_ptr<ir::Func>>>,
                                                        std::optional<std::deque<Cache::FunctionStore>>,
                                                        std::optional<std::shared_ptr<ir::Module>>,
                                                        bool /* Accept private members */> {
    if (auto x = utils::cast<types::ReferenceType>(type)) { type = x->getBaseType(); }
    if (auto x = utils::cast<types::TypeAlias>(type)) { type = x->getBaseType(); }

    if (auto x = utils::cast<types::DefinedType>(type)) {
      auto g = utils::cast<Expression::GenericIdentifier>(index->getIdentifier());
      auto name = index->getIdentifier()->getIdentifier();
      auto generics = (g != nullptr) ? g->getGenerics() : std::vector<Expression::TypeRef*>{};

      auto fullUUID = x->getUUID();
      auto [v, ty, fns, ovs, mod] = getFromIdentifier(dbgInfo, name, generics, fullUUID);

      std::shared_ptr<ir::Value> indexValue = nullptr;
      if (!isStatic) {
        auto fields = x->getFields();
        bool fieldFound = false;
        auto fieldValue = std::find_if(fields.begin(), fields.end(), [&](types::DefinedType::ClassField* f) {
          bool e = f->name == name;
          fieldFound = e;
          return fieldFound;
        });

        if (fieldFound) {
          // assert(v == std::nullopt);
          assert(value != nullptr);

          indexValue =
                  builder.createIndexExtract(dbgInfo, value, *fieldValue, std::distance(fields.begin(), fieldValue));
          builder.setType(indexValue, (*fieldValue)->type);
        }
      }

      if (indexValue == nullptr && v.has_value()) { indexValue = v.value(); }
      if (!indexValue && !ty.has_value() && !fns.has_value() && !ovs.has_value() && !mod.has_value()) {
        if (OperatorService::isOperator(name)) name = OperatorService::operatorName(OperatorService::operatorID(name));
        E<VARIABLE_ERROR>(dbgInfo,
                          FMT("Coudn't find '%s' inside type '%s'!", name.c_str(), x->getPrettyName().c_str()));
      }
      return {indexValue ? std::make_optional(indexValue) : std::nullopt, ty, fns, ovs, mod, isInClassContext(x)};
    } else {
      // Case: index from a (for example) constant. You can access
      //  values from it so just check for functions
      auto name = index->getIdentifier()->getIdentifier();
      auto g = utils::cast<Expression::GenericIdentifier>(index->getIdentifier());
      auto generics = (g != nullptr) ? g->getGenerics() : std::vector<Expression::TypeRef*>{};

      if (type->getName() == _SNOWBALL_CONST_PTR) {
        DUMP_S("HERE")
      }
      auto uuid = type->getName();
      if (auto x = utils::cast<types::PointerType>(type)) {
        auto str = getPointerTypeUUID(x);
        if (!str.empty()) uuid = str;
      }

      auto [v, ty, fns, ovs, mod] = getFromIdentifier(dbgInfo, name, generics, uuid);

      if ((!fns.has_value()) && (!ovs.has_value())) {
        if (OperatorService::isOperator(name)) 
          name = OperatorService::operatorName(OperatorService::operatorID(name));
        E<VARIABLE_ERROR>(
                dbgInfo,
                FMT("Coudn't find function '%s' inside type '%s'!", name.c_str(), type->getPrettyName().c_str()));
      }

      return {std::nullopt, std::nullopt, fns, ovs, std::nullopt, false};
    }

    return {std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, false};
  };

  auto getFromModule =
          [&](std::shared_ptr<ir::Module> m) -> std::tuple<std::optional<std::shared_ptr<ir::Value>>,
                                                           std::optional<types::Type*>,
                                                           std::optional<std::deque<std::shared_ptr<ir::Func>>>,
                                                           std::optional<std::deque<Cache::FunctionStore>>,
                                                           std::optional<std::shared_ptr<ir::Module>>,
                                                           bool /* Accept private members */> {
    // TODO: dont allow operators for modules

    auto g = utils::cast<Expression::GenericIdentifier>(index->getIdentifier());
    auto generics = (g != nullptr) ? g->getGenerics() : std::vector<Expression::TypeRef*>{};

    auto fullUUID = m->getUniqueName();
    auto [v, ty, fns, ovs, mod] =
            getFromIdentifier(dbgInfo, index->getIdentifier()->getIdentifier(), generics, fullUUID);

    if (!v.has_value() && !ty.has_value() && !fns.has_value() && !ovs.has_value() && !mod.has_value()) {
      E<VARIABLE_ERROR>(dbgInfo,
                        FMT("Coudn't find '%s' inside module '%s'!",
                            index->getIdentifier()->getIdentifier().c_str(),
                            m->getName().c_str()));
    }

    return {v, ty, fns, ovs, mod, isInModuleContext(m)};
  };

  // Transform the base first
  auto base = index->getBase();
  if (auto baseIdentifier = utils::cast<Expression::Identifier>(base)) {
    auto [val, type, funcs, overloads, mod] = getFromIdentifier(baseIdentifier);

    if (val && (!isStatic)) {
      return {getFromType(val.value()->getType(), *val), val.value()};
    } else if (val) {
      E<TYPE_ERROR>(dbgInfo,
                    "Static method call / accesses can only be "
                    "used with types, not values!");
    }

    else if (mod && (!isStatic)) {
      E<TYPE_ERROR>(dbgInfo,
                    "Module members must be accessed by using "
                    "static indexes!");
    } else if (mod) {
      return {getFromModule(*mod), std::nullopt};
    }

    else if (type && (!isStatic)) {
      E<TYPE_ERROR>(dbgInfo,
                    "Can't use type references for method calls / "
                    "accesses!");
    } else if (type) {
      return {getFromType(*type), std::nullopt};
    }

    else if (overloads || funcs) {
      E<TYPE_ERROR>(dbgInfo, "Can't use function pointer as index base!");
    } else {
      E<VARIABLE_ERROR>(baseIdentifier->getDBGInfo(),
                        FMT("Cannot find identifier `%s`!", baseIdentifier->getIdentifier().c_str()),
                        {.info = "this name is not defined"});
    }

    assert(false && "BUG: unhandled index value");
  } else if (auto x = utils::cast<Expression::Index>(base)) {
    auto [r, b] = getFromIndex(base->getDBGInfo(), x, x->isStatic);
    auto [v, t, fs, ovs, mod, c] = r;

    if (v && (!isStatic)) {
      return {getFromType(v.value()->getType(), *v), v.value()};
    } else if (v) {
      E<TYPE_ERROR>(dbgInfo,
                    "Static method index can only be "
                    "used with types, not values!");
    }

    else if (mod && (!isStatic)) {
      E<TYPE_ERROR>(dbgInfo,
                    "Module members must be accessed by using "
                    "static indexes!");
    } else if (mod) {
      return {getFromModule(*mod), std::nullopt};
    }

    else if (t && (!isStatic)) {
      E<TYPE_ERROR>(dbgInfo,
                    "Can't use type references for method calls / "
                    "accesses!");
    } else if (t) {
      return {getFromType(*t), std::nullopt};
    }

    else if (ovs || fs) {
      E<TYPE_ERROR>(dbgInfo, "Can't use function pointer as index base!");
    } else {
      // TODO: include base name
      E<VARIABLE_ERROR>(dbgInfo, FMT("Identifier '%s' not found!", baseIdentifier->getIdentifier().c_str()));
    }

    assert(false && "TODO: index index");
  } else if (auto x = utils::cast<Expression::TypeRef>(base)) {
    auto ty = transformType(x);

    if (ty && (!isStatic)) {
      E<TYPE_ERROR>(dbgInfo,
                    "Can't use type references for method calls / "
                    "accesses!");
    } else if (ty) {
      return {getFromType(ty), std::nullopt};
    }
  } else if (!isStatic) {
    // case: Integers, Strings, etc... index
    auto v = trans(base);
    return {getFromType(v->getType(), v), v};
  } else {
    E<SYNTAX_ERROR>(dbgInfo,
                    "Static acces/method call can only be used with "
                    "indentifiers!");
  }

  return {{std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, false}, std::nullopt};
}
} // namespace Syntax
} // namespace snowball