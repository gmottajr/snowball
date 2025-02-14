
#include "../../utils/utils.h"
#include "LLVMBuilder.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

namespace snowball {
namespace codegen {

llvm::Function* LLVMBuilder::getGlobalCTOR(bool createIfNone) {
  auto mangle = (std::string) "_GLOBAL__I" + ".GlobalInit.Snowball:" + iModule->getUniqueName();
  auto fn = module->getFunction(mangle);
  if ((!fn) && createIfNone) {
    auto prototype = llvm::FunctionType::get(builder->getVoidTy(), {});
    fn = h.create<llvm::Function>(prototype, llvm::Function::InternalLinkage, mangle, *module);
#ifdef __linux__
    fn->setSection(".text.startup");
#elif __APPLE__
    fn->setSection("__TEXT,__text");
#endif
    fn->setCallingConv(llvm::CallingConv::C);
    auto file = dbg.getFile(iModule->getSourceInfo()->getPath());
    auto subroutineType = dbg.builder->createSubroutineType(llvm::MDTuple::get(*context, {}));
    auto subprogram = dbg.builder->createFunction(
                        file,
                        FMT("Global constructor for module %s", iModule->getName().c_str()),
                        mangle,
                        file,
                        0,
                        llvm::cast<llvm::DISubroutineType>(subroutineType),
                        /*ScopeLine=*/0,
                        llvm::DINode::FlagPrototyped,
                        llvm::DISubprogram::toSPFlags(
                          /*IsLocalToUnit=*/true,
                          /*IsDefinition=*/true,
                          /*IsOptimized=*/!dbg.debug
                        )
                      );
    fn->setSubprogram(subprogram);
    // TODO: set di info
  } else if (fn) {
    return fn;
  } else if (!fn && (!createIfNone)) {
    return nullptr;
  }
  h.create<llvm::BasicBlock>(builder->getContext(), "body", fn);
  llvm::appendToGlobalCtors(*module, fn, 65535);
  return fn;
}

} // namespace codegen
} // namespace snowball
