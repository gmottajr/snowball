
#include "Func.h"

#include "../../services/OperatorService.h"
#include "../../utils/utils.h"
#include "Argument.h"

#include <string>

namespace snowball {
namespace ir {

Func::Func(std::string identifier, bool declaration, bool variadic, bool isAnon, types::DefinedType* parent)
    : declaration(declaration), variadic(variadic), identifier(identifier), parent(parent), anon(isAnon) { }

Func::Func(
        std::string identifier,
        Func::FunctionArgs arguments,
        bool declaration,
        bool variadic,
        bool isAnon,
        types::DefinedType* parent
)
    : declaration(declaration), variadic(variadic), anon(isAnon), identifier(identifier), parent(parent) {
  setArgs(arguments);
}

Func::Func(
        std::string identifier, std::shared_ptr<Block> body, bool declaration, bool variadic, bool isAnon, types::DefinedType* parent
)
    : declaration(declaration), variadic(variadic), identifier(identifier), parent(parent), anon(isAnon) {
  setBody(body);
}

Func::Func(
        std::string identifier,
        std::shared_ptr<Block> body,
        Func::FunctionArgs arguments,
        bool declaration,
        bool variadic,
        bool isAnon,
        types::DefinedType* parent
)
    : declaration(declaration), variadic(variadic), identifier(identifier), parent(parent), anon(isAnon) {
  setBody(body);
  setArgs(arguments);
}

bool Func::isConstructor() const {
  return (services::OperatorService::opEquals<services::OperatorService::CONSTRUCTOR>(identifier)) && hasParent();
}

std::string Func::getIdentifier() { return identifier; }
std::string Func::getName(bool ignoreOperators) {
  if (services::OperatorService::isOperator(identifier) && (!ignoreOperators)) {
    auto op = services::OperatorService::operatorID(identifier);
    return services::OperatorService::operatorName(op);
  }

  return getIdentifier();
}

Func::FunctionArgs Func::getArgs(bool ignoreSelf) const {
  auto argv = arguments;
  if (ignoreSelf && argv.size() > 0 && ((hasParent() && (!_static)) || isConstructor())) { argv.erase(argv.begin()); }

  return argv;
}

bool Func::isExternal(std::string name) { return !utils::startsWith(name, _SN_MANGLE_PREFIX); }

std::string Func::getNiceName() {
  auto base = hasParent() ? (parent->getPrettyName() + "::") : module->isMain() ? "" : module->getName() + "::";
  std::string generics = "";
  if (isGeneric()) {
    generics = "<";
    for (auto g : getGenerics()) {
      generics += g.second->getPrettyName();
      if (g != getGenerics().back()) generics += ", ";
    }
    generics += ">";
  }
  auto name = getName();
  auto n = base + getName() + generics;
  if (utils::startsWith(base, _SNOWBALL_CONST_PTR_DECL) || utils::startsWith(base, _SNOWBALL_MUT_PTR_DECL)) {
    n = base + getName();
  }

  return n;
}

std::string Func::getMangle() {
  if (!externalName.empty()) return externalName;
  if (hasAttribute(Attributes::EXPORT)) {
    auto args = getAttributeArgs(Attributes::EXPORT);
    auto name = args.find("name");
    if (name != args.end()) { return name->second; }
  }
  if (hasAttribute(Attributes::NO_MANGLE)) return getIdentifier();

  // TODO: add class to here
  auto base = hasParent() ? parent->getMangledName() : module->getUniqueName();

  auto name = getIdentifier();
  if (utils::endsWith(name, _SNOWBALL_LAMBDA_FUNCTIONS)) {
    name = name.substr(0, name.size() - (_SNOWBALL_LAMBDA_SIZE + 1)) + ".$LmbdF";
  }

  std::string prefix = (utils::startsWith(base, _SN_MANGLE_PREFIX) ? base : (_SN_MANGLE_PREFIX + base)) + +"&" +
          std::to_string(name.size()) + name // Function name with modules
          + "Cv" + std::to_string(getId());  // disambiguator

  std::string mangledArgs = "Sa"; // Start args tag

  int argCounter = 1;
  for (auto a : arguments) {
    mangledArgs += "A" + std::to_string(argCounter) + a.second->getType()->getMangledName();
    argCounter++;
  }

  std::string mangled = prefix + mangledArgs + "FnE"; // FnE = end of function
  return mangled;
}

} // namespace ir
} // namespace snowball
