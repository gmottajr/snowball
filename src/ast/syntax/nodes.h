
#include "../../DBGSourceInfo.h"
#include "../../common.h"
#include "../../services/OperatorService.h"
#include "../../utils/utils.h"
#include "../types/Type.h"
#include "common.h"

#include <string>
#include <vector>

#ifndef __SNOWBALL_AST_NODES_H_
#define __SNOWBALL_AST_NODES_H_

/**
 * Syntax nodes are a way for snowball to understand
 * what the source code means in it's own "compiled"
 * format.
 *
 * This is sort of an AST (abstract syntax tree) in
 * where the parser creates a syntax tree that will
 * make it much easier to interpret the source code.
 *
 * The nodes are divided into 2 sections. There are
 * the expressions and the statements.
 */
namespace snowball {
namespace Syntax {

class Visitor;

// In snowball, a block is composed of
// a list of nodes.
struct Block : AcceptorExtend<Block, Node> {
    std::vector<Node *> stmts;

  public:
    Block(std::vector<Node *> s) : AcceptorExtend(), stmts(s) {}
    ~Block() noexcept = default;

    // Aceptance for the AST visitor
    ACCEPT()
    // Geter function to fetch the parsed statements
    auto getStmts() { return stmts; }
};

/**
 * An expression in snowball is a value,
 * or anything that executes and ends up being a value.
 *
 *   var x = 1 + 2
 *           ^^^^^
 * In this case, we consider "1 + 2" as an binary operator
 * expression.
 *
 */
namespace Expression {

/**
 * This struct represents a constant value from the AST.
 * A constant is a value that cannot be altered by the program.
 *
 * These are things like: numbers, strings, null, etc... e.g.:
 *   123, "hello", 89.12, null are constant values.
 */
struct ConstantValue : public AcceptorExtend<ConstantValue, Base> {
  public:
    /**
     * This enum allows a constant value to be
     * differentiate from the other ones.
     */
    enum ConstantType {
#include "../../defs/ct.def"
    };

  private:
    /**
     * The value holding the constant as a string.
     * This constant value can be any of the types.
     *
     * note: Constant strings will have the quotes
     * included.
     */
    std::string value;

    // Constant value type to know what is what.
    ConstantType type;

  public:
    using AcceptorExtend::AcceptorExtend;

    ConstantValue(ConstantType type, std::string value)
        : type(type), value(value){};

    /// @return Get constant value
    std::string getValue() { return value; }
    /// @return Get the type that defines this constant.
    ConstantType getType() { return type; }

    ACCEPT()

  public:
    /**
     * @brief Deduce the constant type based on the token type.
     * @return Respective constant type
     */
    static ConstantType deduceType(TokenType ty);
};

/**
 * @brief A representation of a cast for a value to a
 *  certain type.
 * @example of converting an integer to a float 32
 *  1 | 123 as f32
 *    | ^^^ ^^ ^^^ - cast to f32
 *    | |   |___ cast operator
 *    | |_____ value to cast from
 */
struct Cast : public AcceptorExtend<Cast, Base> {
  private:
    /// @brief Value that's needed to be casted
    Base *value;
    /// @brief Result type thats casted to
    TypeRef *type;

  public:
    using AcceptorExtend::AcceptorExtend;

    Cast(Base *value, TypeRef *ty) : type(ty), value(value){};

    /// @return The value to cast
    auto getValue() { return value; }
    /// @brief get the type trying to be casted to
    auto getType() { return type; }

    ACCEPT()
};

/**
 * Representation of a function call. Functions aren't generated
 * until it's being called.
 *
 * @example
 *   f(x) -> we know the number/types of arguments, and the function
 *           name. We can use this information in order to deduce
 *           what function we should use.
 */
struct FunctionCall : public AcceptorExtend<FunctionCall, Base> {
    /**
     * The callee value that's going to be called.
     * Unlike most programming languages, function calls dont
     * require an identifier because it can just call any expression
     * that ends up being a function and has a respective function type.
     * @example for correct usage
     *    myFunction()
     *    (cond ? myFunc : mySecondFunc)()
     *    myFunction()()
     * @example for incorrect usage
     *    4()
     *    <void return>()
     *    ^---------- '4' or 'type void' are not functions
     */
    Base *callee;

    /// @brief A list of arguments passed to the function
    std::vector<Base *> arguments;

  public:
    using AcceptorExtend::AcceptorExtend;

    FunctionCall(Base *callee, std::vector<Base *> arguments = {})
        : callee(callee), arguments(arguments), AcceptorExtend(){};

    /// @return Get function's callee
    auto getCallee() { return callee; }
    /// @return Call instruction arguments
    auto& getArguments() { return arguments; }
    /// @brief Set a new call expression to this call
    void setCallee(Base *c) { callee = c; }

  public:
    /// @return string representation of a function call arguments
    static std::string
    getArgumentsAsString(const std::vector<std::shared_ptr<types::Type>> args);

    ACCEPT()
};

/**
 * @brief It represents the "new" operator to create a new
 *  instance of a class.
 * @note What it actually does it just calls the new operator
 *  as a static function and creates a new allocation of that
 *  type.
 * @example
 *   _c = alloca [type]
 *   c = [type]::[new operator](_c)
 */
struct NewInstance : public AcceptorExtend<Cast, Base> {
  private:
    /// @brief Value that's needed to be casted
    FunctionCall *call;
    /// @brief Result type thats casted to
    TypeRef *type;

  public:
    using AcceptorExtend::AcceptorExtend;

    NewInstance(FunctionCall *call, TypeRef *ty) : type(ty), call(call){};

    /// @return Get the call value from the operator
    auto getCall() { return call; }
    /// @brief Get the type trying to be initialized
    auto getType() { return type; }

    ACCEPT()
};

/**
 * Representation of an identifier. An identifier can be used to get
 * the value of a variable, reference a class/function, etc...
 */
struct Identifier : public AcceptorExtend<Identifier, Base> {
    // The value used as identifier
    std::string identifier;

  public:
    // using AcceptorExtend::AcceptorExtend;

    Identifier(const std::string& identifier)
        : identifier(identifier), AcceptorExtend(){};

    /// @return Get respective identifier Syntax::Expression::Identifierlue
    auto getIdentifier() { return identifier; }
    virtual std::string getNiceName() const {
        if (utils::startsWith(identifier, "#")) {
            auto i = services::OperatorService::operatorID(identifier);
            return services::OperatorService::operatorName(i);
        }

        return identifier;
    }

    virtual ~Identifier() noexcept = default;
    ACCEPT()
};

/**
 * @brief pseudo string variables are substituted at compile time.
 *  These can be used as "information" variables that can be very
 *  beneficial for the user.
 */
struct PseudoVariable : public AcceptorExtend<PseudoVariable, Base> {
    std::string identifier;

  public:
    PseudoVariable(std::string identifier);
    /// @return Get the pseudo variable identifier without the prefix
    auto getIdentifier() { return identifier; }

    ACCEPT()
};

/**
 * Representation of an index node. Index nodes can either be
 * static or not. It's decided by it's syntax. Using '.' means it's
 * not static but using '::' does imply that it's static.
 */
struct Index : public AcceptorExtend<Index, Base> {
    /// Base value from where we are geting the value
    Base *base;
    /// Identifier node that we are trying to extract.
    /// @note Identifier can also have generics!
    Identifier *identifier;

  public:
    /// @brief Wether the extract is marked as static or not
    bool const isStatic = false;

    using AcceptorExtend::AcceptorExtend;

    Index(Base *base, Identifier *identifier, bool isStatic = false)
        : base(base), isStatic(isStatic), identifier(identifier),
          AcceptorExtend(){};

    /// @return Get respective base value
    auto getBase() { return base; }
    /// @return Get respective base value
    auto getIdentifier() { return identifier; }

    ACCEPT()
};

/**
 * This is the same as an identifier except that it will contain generic
 */
struct GenericIdentifier
    : public AcceptorExtend<GenericIdentifier, Identifier> {

    /// @brief Generics stored into the identifier
    std::vector<Expression::TypeRef *> generics;

  public:
    using AcceptorExtend::AcceptorExtend;
    GenericIdentifier(const std::string& idnt,
                      std::vector<Expression::TypeRef *> generics = {})
        : AcceptorExtend<GenericIdentifier, Identifier>(idnt),
          generics(generics){};

    /// @return generic list set to this identifier
    std::vector<Expression::TypeRef *> getGenerics() const;
    std::string getNiceName() const override;

    ACCEPT()
};

/**
 * @struct BinaryOp
 * @brief Represents a binary operator expression node.
 * @extends AcceptorExtend<BinaryOp, Base>
 */
struct BinaryOp : public AcceptorExtend<BinaryOp, Base> {
    using OpType = services::OperatorService::OperatorType;

    Base *left;         ///< Left node
    Base *right;        ///< Right node
    OpType op_type;     ///< The type of operator
    bool unary = false; ///< Whether it's a unary operator

    /**
     * @brief Determines if the operator is an assignment operator.
     * @param p_node The operator expression node.
     * @return Whether it's an assignment operator.
     */
    static bool is_assignment(BinaryOp *p_node);
    /**
     * @brief Converts the operator type to a string.
     * @return The string representation of the operator.
     */
    std::string to_string() const;
    ACCEPT()

    BinaryOp(OpType t) : op_type(t) {
        unary = (op_type == OpType::NOT || op_type == OpType::BIT_NOT ||
                 op_type == OpType::UPLUS || op_type == OpType::UMINUS);
    };
    ~BinaryOp() noexcept = default;
};

}; // namespace Expression

/**
 * A statement (aka a meaningful declarative sentence)
 * is used to declare something. This something may be
 * a function, class, import, etc.
 *
 *  - https://en.wikipedia.org/wiki/Statement_(logic)
 */
namespace Statement {
struct Base : public AcceptorExtend<Base, Node> {};

/**
 * This is just a wrapper class for all nodes
 * that require some sort of visibility.
 *
 * E.g. some of this nodes can be:
 *   - modules, functions, classes, ...
 */
struct Privacy {

    // The visibility specifies where can the node
    // be accessed from. For example, a private function
    // inside a class can't be accessed outside from it.
    // @warning keep the order the same, always!
    enum Status { PUBLIC, PRIVATE } status = PRIVATE;

  public:
    Privacy() { status = PRIVATE; }

    Privacy(Status status);
    ~Privacy() noexcept = default;

    /// @return node's status
    Status getPrivacy() const;

    /// @brief Set node's privacy
    void setPrivacy(Status s);

  public:
    /// @brief Convert an integer to a Status
    /// @param p_status Integer to transform (note: it will be inverted)
    /// @return equivalent for Status
    static Status fromInt(int p_status);
};

/**
 * Function definition, check out parseFunction for
 * it's respective rules
 */
struct FunctionDef
    : public AcceptorExtend<FunctionDef, Base>,
      public AcceptorExtend<FunctionDef, AttributeHolder<Attributes::Fn>>,
      public AcceptorExtend<FunctionDef, Privacy>,
      public AcceptorExtend<FunctionDef, GenericContainer> {

    // Function's identifier
    std::string name;
    // Arguments available for the functions
    std::vector<Expression::Param *> args;
    // Function's return type
    Expression::TypeRef *retType;
    // Whether or not function can take an infinite
    // number of arguments
    bool variadic = false;
    /// If the function is declared as static or not.
    bool _static = false;
    // Declaration of wether or not the function is declared
    // as virtual.
    bool _virtual = false;

  public:
    FunctionDef(const std::string name, Privacy::Status prvc = PRIVATE);

    /// @brief Get function's identifier
    std::string getName();
    /// @brief Set a function name
    void setName(const std::string name);

    /// @return iterator to the first arg
    auto argBegin() { return args.begin(); }
    /// @return iterator beyond the last arg
    auto argEnd() { return args.end(); }

    /// @return Argument list
    std::vector<Expression::Param *> getArgs() const;
    /// @brief Set a new argument list to the function
    void setArgs(std::vector<Expression::Param *> p_args);

    /// @return Return type
    Expression::TypeRef *getRetType() const;
    /// @brief Set a return type to the function
    void setRetType(Expression::TypeRef *p_type);

    /// @return Whether or not the function is marked as variadic
    bool isVariadic();
    /// @brief Mark if the function is variadic or not
    void setVariadic(bool v = true);

    /// @return of wether or not the function is declared as virtual.
    bool isVirtual();
    /// @brief Mark if the function if it's virtual or not
    void setVirtual(bool v = true);

    /// @return `true` if the function is declared as static
    bool isStatic();
    /// @brief Declare a function static or not.
    void setStatic(bool s = true);

    /// Check if the function is declared as an extern function
    virtual bool isExtern() { return false; }

    // Set an acceptance call
    ACCEPT()
};

/**
 * AST representation for a variable declaration. Variables
 * can either be mutable or unmutable.
 */
struct VariableDecl : public AcceptorExtend<VariableDecl, Base> {

    /// @brief Variables's identifier
    std::string name;
    /// @brief Function's return type
    Expression::Base *value;
    /// @brief Whether the variable can change or not
    bool _mutable = false;
    /**
     * @brief User defined type
     * @example
     *   let a: f32 = 2
     *   ----- converted ir =
     *   variable
     *      value: cast "2"(i32) -> f32
     *   ----- it can also be written as:
     *   let a = 2 as f32
     */
    Expression::TypeRef *definedType = nullptr;

  public:
    VariableDecl(const std::string& name, Expression::Base *value = nullptr,
                 bool isMutable = false);

    /// @brief Get the identifier assign to the variable
    std::string getName() const;
    /// @return the value holt by the variable
    Expression::Base *getValue();
    /// @return Wether or not the variable is mutable
    bool isMutable();
    /// @return The desired type the programmer wants
    /// @note It might possibly be nullptr!
    Expression::TypeRef *getDefinedType();
    /// @brief declare a defined type for the variable
    void setDefinedType(Expression::TypeRef *t);
    /// @return true if the variable has been initialied
    bool isInitialized();

    // Set an acceptance call
    ACCEPT()
};

/**
 * Class definition. Created at "Parser::parseClass" function
 */
struct ClassDef : public AcceptorExtend<ClassDef, Base>,
                  public AcceptorExtend<ClassDef, Privacy>,
                  public AcceptorExtend<ClassDef, GenericContainer> {

    /// @brief Class identifier
    std::string name;
    /// @brief Defined functions to the class
    /// @note Although operators act like functions,
    ///  they are treated differently.
    std::vector<FunctionDef *> functions;
    /// @brief Class defined variables
    std::vector<VariableDecl *> variables;
    /// @brief Class inheritance parent
    Expression::TypeRef *extends = nullptr;

  public:
    ClassDef(std::string name, Expression::TypeRef *extends = nullptr,
             Privacy::Status prvc = PRIVATE);

    /// @brief Get class name
    std::string getName() const;

    /// Add a function to the function list
    void addFunction(FunctionDef *fnDef);
    /// Declare a variable and store it to this class
    void addVariable(VariableDecl *var);

    /// Iterator utilities
    using FunctionIterator = std::vector<FunctionDef *>::iterator;
    using VariableIterator = std::vector<VariableDecl *>::iterator;

    /// @return A full list of declared functions
    std::vector<FunctionDef *>& getFunctions();
    /// @return All variables defined on the current class
    std::vector<VariableDecl *>& getVariables();

    FunctionIterator funcStart();
    FunctionIterator funcEnd();

    VariableIterator varStart();
    VariableIterator varEnd();

    /// @return the parent class being inherited on
    Expression::TypeRef *getParent() const;

    // Set an acceptance call
    ACCEPT()
};

/**
 * @brief Representation of a conditional block or "if statement" in the AST.
 * This contains instructions that are executed inside of it if a condition is
 * met, if not, the "else" statement is executed if it exists.
 */
struct Conditional : public AcceptorExtend<Conditional, Base> {

    // Instructions stored inside a block
    Block *insts;
    // the expression to be evaluated
    Expression::Base *cond;
    // The "else" statement block if the condition is false
    Block *elseBlock;

  public:
    explicit Conditional(Expression::Base *cond, Block *insts,
                         Block *elseBlock = nullptr)
        : cond(cond), insts(insts), elseBlock(elseBlock){};

    /// @return body block instructions to execute
    //   if the condition is met
    auto getBlock() { return insts; }
    /// @return the expression to be evaluated
    auto getCondition() { return cond; }

    /// @return Get "else" statement
    auto getElse() { return elseBlock; }

    // Set a visit handler for the generators
    ACCEPT()
};

/**
 * Import statement. Imports functions, classes and symbols from other
 *  files / modules.
 */
struct ImportStmt : public AcceptorExtend<ImportStmt, Base> {
    /// @brief Represents the path trying to be accessed.
    /// It gets translated from the source code, snowball-like
    /// syntax into a vector of strings.
    /// @example of different import statements and how they are
    ///	 represented.
    ///   | use Core::System			            | {"System"}
    ///   | use hello::myPath::secondPath   | {"myPath", "secondPath"}
    ///   | use hello::myPath:..:helloAgain | {"myPath", "..", "helloAgain"}
    /// @note (1): Last path will be checked with all kind of different
    /// supported
    ///  extensions (sn, so, ...) and it's @c exportSymbol will be the name of
    ///  the last path.
    /// @note (2): The user can manually specify the path extension by doing the
    /// following:
    ///   | use myModule::path:path2:myFile(so)::{ myFunc }
    std::vector<std::string> path;
    /// @brief place where searching the path from. This can be used
    ///  so we can decide from what package we need to import this.
    /// @example
    ///  | use Core::System | package = "Core"
    /// @note If the package name is `$` that means that it's being extracted
    ///  from the current module.
    std::string package;
    /// @brief How the declaration is being exported into the current file.
    std::string exportSymbol;

  public:
    /// @brief Representation of the kind of import statement this is.
    /// import statements can have different functionality such as:
    ///  importing certain symbols from a module or importing every symbol
    ///  into the current scope
    /// @example
    ///   DEFAULT: import std:Math
    ///   EXTRACT: import myModule:pathToExtract::{ myFunction, awesomeClass }
    ///   ALL    : import myAmazingModule:myPath::*
    enum ImportType { DEFAULT, EXTRACT, ALL } type = DEFAULT;

  public:
    ImportStmt(const std::vector<std::string> path = {},
               const std::string package = "$", ImportType ty = DEFAULT);

  public:
    /// @brief get the package name where it's imported from
    std::string getPackage() const;
    /// @return The defined path for the package
    std::vector<std::string> getPath() const;
    /// @brief Get the identifier the user wants it imported as
    /// @c exportSymbol
    std::string getExportSymbol() const;

    // Set an acceptance call
    ACCEPT()
};

/**
 * In snowball, a return statement ends the execution of a
 * function, and returns control to the calling function.
 *
 * @note return type must match function's return type
 */
struct Return : public AcceptorExtend<Return, Base> {

    // Function's return type
    Expression::Base *value = nullptr;

  public:
    Return(Expression::Base *value);

    /// @return the value holt by the variable
    Expression::Base *getValue() const;

    // Set an acceptance call
    ACCEPT()
};

/**
 * This represents a function declaration
 * that has been declared as external.
 *
 * In snowball, external functions are the ones
 * who's names isn't mangled and body is not defined.
 *
 * There can be different type of extern types. E.g.
 * you can have; "C" or "System".
 */
struct ExternFnDef : public AcceptorExtend<ExternFnDef, FunctionDef> {

    // External function for the function.
    std::string externalName;

  public:
    using AcceptorExtend::AcceptorExtend;

    template <class... Args>
    ExternFnDef(std::string externalName, Args&...args)
        : externalName(externalName),
          AcceptorExtend(std::forward<Args>(args)...){};

    // Whether the function is external, it is true in this case
    virtual bool isExtern() override { return true; }
    /// @return Get external function's name.
    std::string getExternalName() { return externalName; }
};

/**
 * A bodied function is a function with a declared block.
 * This block contains the instructions that will be
 * executed once this function is called.
 */
struct BodiedFunction : public AcceptorExtend<BodiedFunction, FunctionDef> {

    // Function's block. This block contains all the intructions
    // a function executes when it's called.
    Block *block;

  public:
    using AcceptorExtend::AcceptorExtend;

    template <class... Args>
    BodiedFunction(Block *block, Args&...args)
        : block(block), AcceptorExtend(std::forward<Args>(args)...){};

    /// @return Get function's body declaration.
    Block *getBody() { return block; }
};

/**
 * An LLVM defined function is a function with a declared LLVM block.
 * This block contains the instructions that will be
 * executed once this function is called.
 */
struct LLVMFunction : public AcceptorExtend<LLVMFunction, FunctionDef> {

    // Function's block. This block contains all the LLVM IR intructions
    // a function executes when it's called.
    std::string block;

  public:
    using AcceptorExtend::AcceptorExtend;

    template <class... Args>
    LLVMFunction(std::string block, Args&...args)
        : block(block), AcceptorExtend(std::forward<Args>(args)...){};

    /// @return Get function's body declaration.
    auto getBody() { return block; }
};

}; // namespace Statement

namespace Expression {
/**
 * A struct representing a Lambda Function, which is derived from the
 * AcceptorExtend class and inherits the BodiedFunction class. It provides a way
 * to define an anonymous function that can be passed as an argument to another
 * function or assigned to a variable.
 *
 * The LambdaFunction struct inherits the functionality of the AcceptorExtend
 * and BodiedFunction classes, allowing it to be used as an acceptor of visitors
 * and to have a body that can be evaluated when called.
 */
struct LambdaFunction
    : public AcceptorExtend<LambdaFunction, Expression::Base> {
  public:
    using AcceptorExtend::AcceptorExtend;

    /* Function used inside the lambda since a lambda struct is
      sor of used as an "Interface" for expressions */
    Statement::BodiedFunction *func = nullptr;

    /// @return the function assigned to this lambda interface
    auto getFunc() { return func; }

    LambdaFunction(Statement::BodiedFunction *func)
        : AcceptorExtend(), func(func){};

    ACCEPT()
};
} // namespace Expression

/**
 * @brief Utility function to create a new node
 * @tparam ...Args arguments for the node
 * @tparam Inst Node type to be initialized
 * @return Resultant node
 */
template <typename Inst, class... Args> Inst *N(Args&&...args) {

    // Our template parameter must
    // be inherited from Node
    static_assert(std::is_base_of<Node, Inst>::value,
                  "Inst must inherit from Node");

    auto n = new Inst(std::forward<Args>(args)...);
    return n;
}

/**
 * @brief Utility function to create a type reference
 * @tparam ...Args arguments for the type
 * @return New type reference
 */
template <class... Args> Expression::TypeRef *TR(Args&&...args) {
    auto n = new Expression::TypeRef(std::forward<Args>(args)...);
    return n;
}

}; // namespace Syntax
}; // namespace snowball

#undef ACCEPT
#endif // __SNOWBALL_AST_NODES_H_
