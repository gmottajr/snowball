
#include "../../errors.h"
#include "../../ir/values/Argument.h"
#include "../../utils/utils.h"
#include "LLVMBuilder.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/SourceMgr.h>

namespace snowball {
namespace codegen {

llvm::Function* LLVMBuilder::buildLLVMFunction(llvm::Function* llvmFn, ir::Func* fn) {
  // auto existant = module->getFunction(fn->getMangle());
  // if (existant) return existant;
  ctx->setCurrentFunction(nullptr);

  // llvmFn->getDe
  std::string bufStr;
  llvm::raw_string_ostream buf(bufStr);

  buf << "define ";
  llvmFn->getReturnType()->print(buf);
  buf << " @\"" << llvmFn->getName() << "\"(";

  auto args = fn->getArgs();
  for (int i = 0; i < args.size(); i++) {
    auto l_front = args.begin();
    std::advance(l_front, i);

    llvmFn->getArg(i)->getType()->print(buf);
    buf << " %" << l_front->first;

    if (i < (args.size() - 1)) buf << ", ";
  }

  buf << ") { \nentry:\n\t";
  buf << fn->getLLVMBody();
  buf << "\n}";

  auto code = buf.str();
  bufStr.clear();

  auto llvmBuffer = llvm::MemoryBuffer::getMemBuffer(code);

  llvm::SMDiagnostic err;
  std::unique_ptr<llvm::Module> sub = llvm::parseIR(llvmBuffer->getMemBufferRef(), err, *context);
  if (!sub) {
    std::string bufStr;
    llvm::raw_string_ostream buf(bufStr);
    err.print("LLVM", buf);
    Logger::error("Error generating user-defined llvm function");
    for (auto line : utils::split(code, "\n")) { Logger::error(line); }

    throw SNError(Error::LLVM_INTERNAL, buf.str());
  }

  sub->setDataLayout(module->getDataLayout());

  llvm::Linker linker(*module);
  const bool fail = linker.linkInModule(std::move(sub));
  assert(!fail);

  auto func = module->getFunction(fn->getMangle());
  assert(func && "function not linked in");
  func->setLinkage(llvm::GlobalValue::PrivateLinkage);
  func->addFnAttr(llvm::Attribute::AttrKind::NoInline); // TODO: user decides,
                                                        // this is default
  func->setSubprogram(getDISubprogramForFunc(fn));

  if (utils::cast<types::ReferenceType>(fn->getRetTy())) {
    auto bytes = module->getDataLayout().getTypeSizeInBits(getLLVMType(fn->getRetTy()));
    auto dereferenceable = llvm::Attribute::get(*context, llvm::Attribute::Dereferenceable, bytes);
    auto noundef = llvm::Attribute::get(*context, llvm::Attribute::NoUndef);
    auto nonnull = llvm::Attribute::get(*context, llvm::Attribute::NonNull);
    auto aligment = llvm::Attribute::get(*context, llvm::Attribute::Alignment, 8);
    //llvmFn->addRetAttr(dereferenceable);
    llvmFn->addRetAttr(noundef);
    llvmFn->addRetAttr(aligment);
    llvmFn->addRetAttr(nonnull);
  }

  llvmFn->replaceAllUsesWith(func);
  llvmFn->eraseFromParent();
  setDebugInfoLoc(nullptr);
  return func;
}

} // namespace codegen
} // namespace snowball
