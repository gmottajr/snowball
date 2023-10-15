#include "../../Transformer.h"

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

// Set the default '=' operator for the class
#define GENERATE_EQUALIZERS                                                                                            \
  if (ty->getName() != _SNOWBALL_CONST_PTR && ty->getName() != _SNOWBALL_MUT_PTR &&                                    \
      ty->getName() != _SNOWBALL_INT_IMPL) {                                                                           \
    for (int allowPointer = 0; allowPointer < 2; ++allowPointer) {                                                     \
      auto fn = Syntax::N<Statement::FunctionDef>(                                                                     \
              OperatorService::getOperatorMangle(OperatorType::EQ), Statement::Privacy::Status::PUBLIC                 \
      );                                                                                                               \
      fn->addAttribute(Attributes::BUILTIN);                                                                           \
      fn->setArgs({new Expression::Param(                                                                              \
              "other", allowPointer ? transformedType->getReferenceTo()->toRef() : transformedType->toRef()            \
      )});                                                                                                             \
      fn->setRetType(ctx->getVoidType()->toRef());                                                                     \
      trans(fn);                                                                                                       \
    }                                                                                                                  \
  }

namespace snowball {
namespace Syntax {

types::BaseType* Transformer::transformClass(
        const std::string& uuid, cacheComponents::Types::TypeStore& classStore, Expression::TypeRef* typeRef
) {
  auto ty = utils::cast<Statement::DefinedTypeDef>(classStore.type);
  assert(ty);

  // These are the generics generated outside of the class context.
  // for example, this "Test" type woudn't be fetched inside the class
  // context:
  //
  //   class Hello<T> { ... }
  //   Hello<?Test> // Test is not being transformed from the "Hello
  //   context".
  //
  // Note that the default class generics WILL be generated inside the
  // class context.
  auto generics = typeRef != nullptr ? vector_iterate<Expression::TypeRef*, types::Type*>(
                                               typeRef->getGenerics(), [&](auto t) { return transformType(t); }
                                       ) :
                                       std::vector<types::Type*>{};

  // TODO: check if typeRef generics match class generics
  types::BaseType* transformedType;
  ctx->withState(classStore.state, [&]() {
    ctx->withScope([&] {
      auto tyFunctions = ty->getFunctions();
      auto backupClass = ctx->getCurrentClass();
      // TODO: maybe not reset completly, add nested classes in
      // the future
      ctx->setCurrentClass(nullptr);
      auto baseUuid = ctx->createIdentifierName(ty->getName());
      auto existantTypes = ctx->cache->getTransformedType(uuid);
      auto _uuid = baseUuid + ":" + utils::itos(existantTypes.has_value() ? existantTypes->size() : 0);
      auto basedName = ty->getName();
      transformedType = ty->isInterface() ? (types::BaseType*)new types::InterfaceType(
              basedName,
              _uuid,
              ctx->module,
              std::vector<types::InterfaceType::Member*>{},
              std::vector<types::Type*>{}
      ) : (types::BaseType*)new types::DefinedType(
              basedName,
              _uuid,
              ctx->module,
              ty,
              std::vector<types::DefinedType::ClassField*>{},
              nullptr,
              std::vector<types::Type*>{},
              ty->isStruct()
      );
      transformedType->addImpl(ctx->getBuiltinTypeImpl("Sized"));
      auto item = std::make_shared<transform::Item>(transformedType);
      ctx->cache->setTransformedType(baseUuid, item, _uuid);
      auto classGenerics = ty->getGenerics();
      auto selfType = std::make_shared<Item>(transformedType);
      ctx->addItem("Self", selfType);
      for (int genericCount = 0; genericCount < generics.size(); genericCount++) {
        auto generic = classGenerics.at(genericCount);
        auto generatedGeneric = generics.at(genericCount);
        auto item = std::make_shared<transform::Item>(generatedGeneric->copy());
        // TODO:
        // item->setDBGInfo(generic->getDBGInfo());
        ctx->addItem(generic->getName(), item);
        executeGenericTests(generic->getWhereClause(), generatedGeneric, generic->getName());
      }
      // Fill out the remaining non-required tempalte parameters
      if (classGenerics.size() > generics.size()) {
        for (auto i = generics.size(); i < classGenerics.size(); ++i) {
          auto generic = classGenerics[i];
          auto generatedGeneric = transformType(generic->type);
          auto item = std::make_shared<transform::Item>(generatedGeneric->copy());
          // TODO:
          // item->setDBGInfo(generic->getDBGInfo());
          ctx->addItem(generic->getName(), item);
          executeGenericTests(generic->getWhereClause(), generatedGeneric, generic->getName());
        }
      }
      types::DefinedType* parentType = nullptr;
      if (auto x = ty->getParent()) {
        auto parent = transformSizedType(x, false, "Parent types must be sized but found '%s' (which is not sized)");
        parentType = utils::cast<types::DefinedType>(parent);
        if (!parentType) {
          E<TYPE_ERROR>(
                  x,
                  FMT("Can't inherit from '%s'", parent->getPrettyName().c_str()),
                  {.info = "This is not a class nor a struct type!",
                   .note = "Classes can only inherit from other "
                           "classes or "
                           "structs meaning\n that you can't "
                           "inherit from `i32` "
                           "(for example) because it's\n a "
                           "primitive type.",
                   .help = "If trying to implement from an interface, "
                           "use the `implements`\nkeyword "
                           "instead.\n"}
          );
        }
      }
      auto backupGenerateFunction = ctx->generateFunction;
      ctx->setCurrentClass(transformedType);
      // Transform types first thing
      ctx->generateFunction = false;
      for (auto ty : ty->getTypeAliases()) { trans(ty); }
      ctx->generateFunction = true;
      for (auto ty : ty->getTypeAliases()) { trans(ty); }
      transformedType->setGenerics(generics);
      transformedType->setPrivacy(ty->getPrivacy());
      transformedType->setDBGInfo(ty->getDBGInfo());
      transformedType->setSourceInfo(ty->getSourceInfo());
      int fieldCount = 0;
      if (!ty->isInterface()) {
        auto baseFields = vector_iterate<Statement::VariableDecl*, types::DefinedType::ClassField*>(
          ty->getVariables(),
          [&](auto v) {
            auto definedType = v->getDefinedType();
            if (!definedType)
              E<SYNTAX_ERROR>(
                v->getDBGInfo(),
                "Can't infer type!",
                {.info = "The type of this variable can't be inferred!",
                  .note = "This rule only applies to variables inside classes.",
                  .help = "You can't infer the type of a variable "
                    "without specifying it's type.\n"
                    "For example, you can't do this:\n   let a = 10\n"
                    "You have to do this:\n   let a: i32 = 10\n"
                    "Or this:\n   let a = 10: i32"}
              );
            auto varTy = transformSizedType(
                    definedType, false, "Class fields must be sized but found '%s' (which is not sized)"
            );
            varTy->setMutable(v->isMutable());
            auto field = new types::DefinedType::ClassField(
                    v->getName(), varTy, v->getPrivacy(), v->getValue(), v->isMutable()
            );
            field->setDBGInfo(v->getDBGInfo());
            return field;
          }
        );
        auto fields = getMemberList(ty->getVariables(), baseFields, parentType);
        ((types::DefinedType*)transformedType)->setParent(parentType);
        ((types::DefinedType*)transformedType)->setFields(fields);
        fieldCount = fields.size();
      } else {
        for (auto v : ty->getVariables()) {
          auto definedType = v->getDefinedType();
          if (!definedType)
            E<SYNTAX_ERROR>(
              v->getDBGInfo(),
              "Can't infer type!",
              {.info = "The type of this variable can't be inferred!",
                .note = "This rule only applies to variables inside interfaces.",
                .help = "You can't infer the type of a variable "
                  "without specifying it's type.\n"
                  "For example, you can't do this:\n   let a = 10\n"
                  "You have to do this:\n   let a: i32 = 10\n"
                  "Or this:\n   let a = 10: i32"}
            );
          auto varTy = transformSizedType(
                  definedType, false, "Interface fields must be sized but found '%s' (which is not sized)"
          );
          varTy->setMutable(v->isMutable());
          auto field = new types::InterfaceType::Member(
                  v->getName(), varTy, types::InterfaceType::Member::Kind::FIELD, v, v->getPrivacy(), v->isMutable()
          );
          field->setDBGInfo(v->getDBGInfo());
          ((types::InterfaceType*)transformedType)->addField(field);
          fieldCount++;
        }
      }
      bool allowConstructor = (!ty->hasConstructor) && fieldCount == 0;
      if (parentType != nullptr && !ty->isInterface()) 
        ctx->cache->performInheritance((types::DefinedType*)transformedType, parentType, allowConstructor);
      if (!ty->isInterface())
        implementTypes((types::DefinedType*)transformedType, ty->getImpls(), tyFunctions);
      assert(!ty->isStruct() || (ty->isStruct() && ty->getFunctions().size() == 0));
      // Create function definitions
      ctx->generateFunction = false;
      if (!ty->isInterface()) {
        types::DefinedType* classType = utils::cast<types::DefinedType>(transformedType);
        ctx->module->typeInformation.insert(
          {classType->getId(), std::shared_ptr<types::DefinedType>(classType)}
        );
        GENERATE_EQUALIZERS
        for (auto fn : tyFunctions) {
          if (services::OperatorService::opEquals<OperatorType::CONSTRUCTOR>(fn->getName()))
            classType->hasConstructor = true;
          if (fn->isVirtual()) classType->hasVtable = true;

          trans(fn);
        }
        if (!classType->hasVtable) {
          auto p = classType;
          while (p->hasParent()) {
            p = p->getParent();
            p = p->getModule()->typeInformation.find(p->getId())->second.get();
            if (p->hasVtable) {
              classType->hasVtable = true;
              break;
            }
          }
        }
        // Generate the function bodies
        ctx->generateFunction = true;
        GENERATE_EQUALIZERS
        for (auto fn : tyFunctions) { trans(fn); }
        ctx->generateFunction = backupGenerateFunction;
        auto parentHasConstructor =
                allowConstructor && parentType != nullptr && !parentType->isStruct() && parentType->hasConstructor;
        if (!parentHasConstructor && !classType->hasConstructor && !classType->isStruct() &&
            !ty->hasAttribute(Attributes::BUILTIN)) {
          E<SYNTAX_ERROR>(
                  ty,
                  "This class does not have a constructor!",
                  {.info = "This class does not have a constructor!",
                  .note = "No constructor has been defined or can be inherited.",
                  .help = FMT("You have to define a constructor for this class.\n"
                          "For example:\n\n"
                          "1 | class %s {\n"
                          "2 |   %s() { ... }\n"
                          "3 | }\n"
                          "4 |", ty->getName().c_str(), ty->getName().c_str())}
          );
        }
      }
      ctx->setCurrentClass(backupClass);
    });
  });
  return transformedType;
}

} // namespace Syntax
} // namespace snowball