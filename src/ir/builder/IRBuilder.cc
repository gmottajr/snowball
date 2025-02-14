
#include "IRBuilder.h"
#include "../../ast/syntax/nodes.h"
#include "../../common.h"
#include "../../sourceInfo/DBGSourceInfo.h"

namespace snowball {
namespace ir {

IRBuilder::IRBuilder(std::shared_ptr<ir::Module> module) { setModule(module); }
SharedValue<Func> IRBuilder::createFunction(DBGSourceInfo* dbgInfo, std::string name, bool isExtern, bool isVarArg,
    bool isAnon) {
  return N<Func>(dbgInfo, name, isExtern, isVarArg, isAnon);
}
SharedValue<Func> IRBuilder::createFunction(
  DBGSourceInfo* dbgInfo, std::string name, Func::FunctionArgs args, bool isExtern, bool isVarArg, bool isAnon
) {
  return N<Func>(dbgInfo, name, args, isExtern, isVarArg, isAnon);
}
SharedValue<Func> IRBuilder::createFunction(
  DBGSourceInfo* dbgInfo,
  std::string name,
  SharedValue<Block> block,
  Func::FunctionArgs args,
  bool isExtern,
  bool isVarArg,
  bool isAnon
) {
  return N<Func>(dbgInfo, name, block, args, isExtern, isVarArg, isAnon);
}
SharedValue<Func> IRBuilder::createFunction(
  DBGSourceInfo* dbgInfo, std::string name, SharedValue<Block> block, bool isExtern, bool isVarArg, bool isAnon
) {
  return createFunction(dbgInfo, name, block, {}, isExtern, isVarArg, isAnon);
}
SharedValue<Throw> IRBuilder::createThrow(DBGSourceInfo* dbgInfo, SharedValue<> value) {
  return N<Throw>(dbgInfo, value);
}
SharedValue<Cast> IRBuilder::createCast(DBGSourceInfo* dbgInfo, SharedValue<> value, Type<> type) {
  auto cast = N<Cast>(dbgInfo, value, type);
  setType(cast, type);
  return cast;
}
SharedValue<IndexExtract> IRBuilder::createIndexExtract(
  DBGSourceInfo* dbgInfo, SharedValue<> value, types::DefinedType::ClassField* field, unsigned int index
) {
  auto indexExtract = N<IndexExtract>(dbgInfo, value, field, index);
  setType(indexExtract, field->type);
  return indexExtract;
}
SharedValue<Argument> IRBuilder::createArgument(DBGSourceInfo* dbgInfo, const std::string& name, Type<> type,
    int scopeIndex) {
  auto arg = createArgument(dbgInfo, name, 0, (Syntax::Expression::Base*) nullptr, scopeIndex);
  if (type) setType(arg, type);
  return arg;
}
SharedValue<Argument> IRBuilder::createArgument(
  DBGSourceInfo* dbgInfo, const std::string& name, int index, Syntax::Expression::Base* defaultValue, int scopeIndex
) {
  return N<Argument>(dbgInfo, name, index, defaultValue, scopeIndex);
}
SharedValue<EnumInit> IRBuilder::createEnumInit(DBGSourceInfo* dbgInfo, types::EnumType* type, std::string name) {
  auto init = N<EnumInit>(dbgInfo, name);
  setType(init, type);
  return init;
}
SharedValue<LoopFlow> IRBuilder::createLoopFlow(
  DBGSourceInfo* dbgInfo, LoopFlowType type) {
  return N<LoopFlow>(dbgInfo, type);
}
SharedValue<ZeroInitialized> IRBuilder::createZeroInitialized(DBGSourceInfo* dbg, Type<> type) {
  auto zero = N<ZeroInitialized>(dbg);
  zero->setType(type); // already copied
  return zero;
}
SharedValue<Argument> IRBuilder::createArgument(
  DBGSourceInfo* dbgInfo, const std::string& name, int index, Type<> type, Syntax::Expression::Base* defaultValue,
  int scopeIndex
) {
  auto arg = createArgument(dbgInfo, name, index, defaultValue, scopeIndex);
  setType(arg, type);
  return arg;
}
SharedValue<Variable>
IRBuilder::createVariable(DBGSourceInfo* dbgInfo, const std::string& identifier, bool isArgument, bool isMutable,
                          int scopeIndex) {
  return N<Variable>(dbgInfo, identifier, isArgument, isMutable, scopeIndex);
}
SharedValue<Variable> IRBuilder::createVariable(
  DBGSourceInfo* dbgInfo, const std::string& identifier, Type<> type, bool isArgument, bool isMutable, int scopeIndex
) {
  auto var = createVariable(dbgInfo, identifier, isArgument, isMutable, scopeIndex);
  setType(var, type);
  return var;
}
SharedValue<Block> IRBuilder::createBlock(DBGSourceInfo* dbgInfo) { return createBlock(dbgInfo, {}); }
SharedValue<Block> IRBuilder::createBlock(DBGSourceInfo* dbgInfo, std::vector<SharedValue<>> values) {
  return N<Block>(dbgInfo, values);
}
SharedValue<DereferenceTo> IRBuilder::createDereferenceTo(DBGSourceInfo* dbgInfo, SharedValue<> value, Type<> type) {
  auto ref = N<DereferenceTo>(dbgInfo, value);
  setType(ref, type);
  return ref;
}
SharedValue<ReferenceTo> IRBuilder::createReferenceTo(DBGSourceInfo* dbgInfo, SharedValue<> value) {
  auto ref = N<ReferenceTo>(dbgInfo, value);
  ref->setType(value->getType()->getReferenceTo());
  ref->getType()->setMutable(value->getType()->isMutable());
  return ref;
}
SharedValue<StringValue> IRBuilder::createStringValue(DBGSourceInfo* dbgInfo, const std::string value) {
  return N<StringValue>(dbgInfo, value);
}
SharedValue<NumberValue> IRBuilder::createNumberValue(DBGSourceInfo* dbgInfo, snowball_int_t value) {
  return N<NumberValue>(dbgInfo, value);
}
SharedValue<FloatValue> IRBuilder::createFloatValue(DBGSourceInfo* dbgInfo, double value) {
  return N<FloatValue>(dbgInfo, value);
}
SharedValue<BooleanValue> IRBuilder::createBooleanValue(DBGSourceInfo* dbgInfo, bool value) {
  return N<BooleanValue>(dbgInfo, value);
}
SharedValue<CharValue> IRBuilder::createCharValue(DBGSourceInfo* dbgInfo, char value) {
  return N<CharValue>(dbgInfo, value);
}
SharedValue<Return> IRBuilder::createReturn(DBGSourceInfo* dbgInfo, SharedValue<> value) {
  auto ret = N<Return>(dbgInfo, value);
  if (value) setType(ret, value->getType());
  return ret;
}
SharedValue<Return> IRBuilder::createVoidReturn(DBGSourceInfo* dbgInfo) { return N<Return>(dbgInfo, nullptr); }
SharedValue<Call> IRBuilder::createCall(DBGSourceInfo* dbgInfo, SharedValue<> callee, ValueVec<> args) {
  return N<Call>(dbgInfo, callee, args);
}
SharedValue<VariableDeclaration> IRBuilder::createVariableDeclaration(
  DBGSourceInfo* dbgInfo, std::shared_ptr<ir::Variable> variable, SharedValue<> value, bool isExtern
) {
  auto decl = N<VariableDeclaration>(dbgInfo, variable, value, isExtern);
  if (value != nullptr) setType(decl, value->getType());
  else setType(decl, variable->getType());
  return decl;
}
SharedValue<ValueExtract> IRBuilder::createValueExtract(DBGSourceInfo* dbgInfo, SharedValue<> value) {
  auto extract = N<ValueExtract>(dbgInfo, value);
  setType(extract, value->getType());
  return extract;
}
SharedValue<TryCatch> IRBuilder::createTryCatch(
  DBGSourceInfo* dbgInfo,
  SharedValue<Block> tryBlock,
  std::vector<SharedValue<Block>> catchBlocks,
  std::vector<SharedValue<VariableDeclaration>> catchVars
) {
  return N<TryCatch>(dbgInfo, tryBlock, catchBlocks, catchVars);
}
SharedValue<Value>
IRBuilder::createObjectInitialization(DBGSourceInfo* dbgInfo, types::Type* type, SharedValue<> value, ValueVec<> args,
                                      bool isStruct) {
  std::shared_ptr<ir::Value> init = N<ObjectInitialization>(dbgInfo, value, args, isStruct);
  setType(init, type);
  return init;
}
SharedValue<Conditional> IRBuilder::createConditional(
  DBGSourceInfo* dbgInfo, SharedValue<> condition, SharedValue<Block> thenBlock, SharedValue<Block> elseBlock
) {
  return N<Conditional>(dbgInfo, condition, thenBlock, elseBlock);
}
SharedValue<WhileLoop>
IRBuilder::createWhileLoop(DBGSourceInfo* dbgInfo, SharedValue<> condition, SharedValue<Block> body, bool isDoWhile) {
  return N<WhileLoop>(dbgInfo, condition, body, isDoWhile);
}
SharedValue<WhileLoop> IRBuilder::createFromForLoop(
  DBGSourceInfo* dbgInfo,
  SharedValue<> condition,
  SharedValue<Block> body,
  SharedValue<> forCond
) {
  return N<WhileLoop>(dbgInfo, condition, body, forCond);
}
Type<types::FunctionType>
IRBuilder::createFunctionType(std::vector<Type<>> args, Type<> retType, bool isVarArg, bool isMutable) {
  return new types::FunctionType(args, retType, isVarArg, isMutable);
}
SharedValue<Switch> IRBuilder::createSwitch(
  DBGSourceInfo* dbgInfo, SharedValue<> expr, std::vector<Switch::Case> cases, SharedValue<Block> defaultBlock
) {
  return N<Switch>(dbgInfo, expr, cases, defaultBlock);
}
SharedValue<Switch> IRBuilder::createSwitch(DBGSourceInfo* dbgInfo, SharedValue<> expr,
    std::vector<Switch::CStyleCase> cases, SharedValue<Block> defaultBlock) {
  return N<Switch>(dbgInfo, expr, cases, defaultBlock);
}
Type<types::TypeAlias> IRBuilder::createTypeAlias(DBGSourceInfo* dbg, std::string name, Type<> base) {
  auto ty = new types::TypeAlias(name, base);
  ty->unsafeSetModule(module);
  ty->setSourceInfo(module->getSourceInfo());
  ty->setDBGInfo(dbg);
  ty->setMutable(base->isMutable());
  return ty;
}
SharedValue<BinaryOp> IRBuilder::createBinaryOp(SharedValue<Call> call) {
  auto op = N<BinaryOp>(call->getDBGInfo(), call->getCallee(), call->getArguments());
  op->isInitialization = call->isInitialization;
  setType(op, call->getType());
  return op;
}
void IRBuilder::setType(SharedValue<> value, Type<> type) { value->setType(type); }
} // namespace ir
} // namespace snowball