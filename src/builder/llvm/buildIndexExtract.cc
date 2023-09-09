
#include "../../ir/values/IndexExtract.h"
#include "../../utils/utils.h"
#include "LLVMBuilder.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace snowball {
namespace codegen {

void LLVMBuilder::visit(ir::IndexExtract* index) {
  auto indexValue = index->getValue();
  auto valueType = indexValue->getType();
  auto basedType = valueType;
  if (auto x = utils::cast<types::ReferenceType>(basedType)) basedType = x->getPointedType();
  if (auto x = utils::cast<types::PointerType>(basedType)) basedType = x->getPointedType();
  auto defiendType = utils::cast<types::DefinedType>(basedType);
  assert(defiendType);

  // We add "1" becasue index #0 is a pointer to the virtual
  // table.
  // TODO: support for structs without vtable.
  // we do ctx->getVtableTy instead of x->hasVtable to avoid any issues when cloning
  auto i = index->getIndex() + (ctx->getVtableTy(defiendType->getId()) != nullptr);

  auto leftArray = build(indexValue.get());
  if (utils::is<types::ReferenceType>(valueType))
    leftArray = builder->CreateLoad(getLLVMType(basedType)->getPointerTo(), leftArray);

  auto extract = builder->CreateStructGEP(getLLVMType(basedType), leftArray, i);
  this->value = extract;
}

} // namespace codegen
} // namespace snowball
