#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace descartes {

struct Symbol {
  explicit Symbol(int id);
  const std::string &getName() const;
  bool operator==(const Symbol &other) const;
  int id;
  std::string *value;
};

struct SymbolHash {
  size_t operator()(const Symbol &sym) const {
    return std::hash<int>()(sym.id);
  }
};

enum class TypeKind {
  Integer,
  Boolean,
  Enum,
  Record,
  Alias,
  String,
};

struct Type {
  virtual ~Type() = default;
  virtual TypeKind getKind() const = 0;
  bool isPointer = false;
};
using TypePtr = std::unique_ptr<Type>;

// TODO: Implement ranged types.
struct Integer : public Type {
  TypeKind getKind() const override { return TypeKind::Integer; }
};

struct Boolean : public Type {
  TypeKind getKind() const override { return TypeKind::Boolean; }
};

struct Enum : public Type {
  explicit Enum(std::vector<Symbol> &&enums) : enums(std::move(enums)) {}
  TypeKind getKind() const override { return TypeKind::Enum; }
  std::vector<Symbol> enums;
};

struct Record : public Type {
  explicit Record(std::vector<std::pair<Symbol, Symbol>> &&fields)
      : fields(std::move(fields)) {}
  TypeKind getKind() const override { return TypeKind::Record; }
  std::vector<std::pair<Symbol, Symbol>> fields;
};

struct Alias : public Type {
  explicit Alias(Symbol typeIdentifier) : typeIdentifier(typeIdentifier) {}
  TypeKind getKind() const override { return TypeKind::Alias; }
  Symbol typeIdentifier;
};

struct String : public Type {
  TypeKind getKind() const override { return TypeKind::String; }
};

enum class ExprKind {
  StringLiteral,
  NumberLiteral,
  VarRef,
  BinaryOp,
  Call,
  MemberRef,
};

class Expr {
public:
  virtual ~Expr() = default;
  virtual ExprKind getKind() const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct ConstDef {
  ConstDef(Symbol identifier, ExprPtr constExpr);
  Symbol identifier;
  ExprPtr constExpr;
};

struct TypeDef {
  TypeDef(Symbol identifier, TypePtr type);
  Symbol identifier;
  TypePtr type;
};

struct VarDecl {
  VarDecl(Symbol identifier, Symbol type);
  Symbol identifier, type;
};

enum class StatementKind {
  Assignment,
  Goto,
  Compound,
  If,
  Case,
  Repeat,
  While,
  For,
  With,
  Call,
};

struct Statement {
  virtual ~Statement() = default;
  virtual StatementKind getKind() const = 0;
};
using StatementPtr = std::unique_ptr<Statement>;

struct Assignment : public Statement {
  Assignment(ExprPtr lhs, ExprPtr rhs);
  StatementKind getKind() const override;
  ExprPtr lhs, rhs;
};

struct Compound : public Statement {
  explicit Compound(std::vector<StatementPtr> body);
  StatementKind getKind() const override;
  std::vector<StatementPtr> body;
};

struct If : public Statement {
  If(ExprPtr cond, StatementPtr thenStatement, StatementPtr elseStatement);
  StatementKind getKind() const override;
  ExprPtr cond;
  StatementPtr thenStatement, elseStatement;
};

struct CaseArm {
  CaseArm(ExprPtr value, StatementPtr statement)
      : value(std::move(value)), statement(std::move(statement)) {}
  ExprPtr value;
  StatementPtr statement;
};

struct Case : public Statement {
  Case(ExprPtr expr, std::vector<CaseArm> &&arms)
      : expr(std::move(expr)), arms(std::move(arms)) {}
  StatementKind getKind() const override;
  ExprPtr expr;
  std::vector<CaseArm> arms;
};

struct Repeat : public Statement {
  Repeat(ExprPtr untilCond, std::vector<StatementPtr> body);
  StatementKind getKind() const;
  ExprPtr untilCond;
  std::vector<StatementPtr> body;
};

struct While : public Statement {
  While(ExprPtr cond, StatementPtr body);
  StatementKind getKind() const override;
  ExprPtr cond;
  StatementPtr body;
};

struct For : public Statement {
  For(Symbol controlIdentifier, ExprPtr begin, ExprPtr end, bool to,
      StatementPtr body);
  StatementKind getKind() const override;
  Symbol controlIdentifier;
  ExprPtr begin, end;
  bool to;
  StatementPtr body;
};

struct With : public Statement {
  With(std::vector<Symbol> &&recordIdentifiers, StatementPtr body);
  StatementKind getKind() const;
  std::vector<Symbol> recordIdentifiers;
  StatementPtr body;
};

struct CallStatement : public Statement {
  explicit CallStatement(ExprPtr call);
  StatementKind getKind() const override;
  ExprPtr call;
};

struct Function;

struct Block {
  Block(std::vector<Symbol> &&labelDecls, std::vector<ConstDef> &&constDefs,
        std::vector<TypeDef> &&typeDefs, std::vector<VarDecl> &&varDecls,
        std::vector<std::unique_ptr<Function>> functions,
        StatementPtr statements);
  std::vector<Symbol> labelDecls;
  std::vector<ConstDef> constDefs;
  std::vector<TypeDef> typeDefs;
  std::vector<VarDecl> varDecls;
  std::vector<std::unique_ptr<Function>> functions;
  StatementPtr statements;
};

struct FunctionArg {
  FunctionArg(Symbol identifier, Symbol type, bool isConst)
      : identifier(identifier), type(type), isConst(isConst) {}
  Symbol identifier, type;
  bool isConst;
};

struct Function {
  Function(Symbol name, std::vector<FunctionArg> &&args, Block &&block,
           std::optional<Symbol> returnType);
  Symbol name;
  std::vector<FunctionArg> args;
  Block block;
  std::optional<Symbol> returnType;
};

struct StringLiteral : public Expr {
  explicit StringLiteral(Symbol val);
  ExprKind getKind() const override;
  Symbol val;
};

struct NumberLiteral : public Expr {
  explicit NumberLiteral(int val);
  ExprKind getKind() const override;
  int val;
};

struct VarRef : public Expr {
  explicit VarRef(Symbol identifier);
  ExprKind getKind() const override;
  Symbol identifier;
};

struct MemberRef : public Expr {
  MemberRef(ExprPtr expr, Symbol identifier);
  ExprKind getKind() const override;
  ExprPtr expr;
  Symbol identifier;
};

struct Call : public Expr {
  Call(Symbol functionName, std::vector<ExprPtr> args);
  ExprKind getKind() const override;
  Symbol functionName;
  std::vector<ExprPtr> args;
};

enum class BinaryOpKind {
  Add,
  Subtract,
  Multiply,
  Divide,
  Equal,
  LessThan,
  GreaterThan,
  NotEqual,
  LessThanEqual,
  GreaterThanEqual,
};

const char *binaryOpKindToString(BinaryOpKind kind);

struct BinaryOp : public Expr {
  BinaryOp(BinaryOpKind kind, ExprPtr lhs, ExprPtr rhs);
  ExprKind getKind() const override;
  BinaryOpKind kind;
  ExprPtr lhs, rhs;
};

template <typename T, ExprKind kind> inline T exprCastImpl(Expr &ast) {
  if (ast.getKind() == kind)
    return static_cast<T>(&ast);
  return nullptr;
}

template <typename T> inline T exprCast(Expr &) {
  static_assert(sizeof(T) != sizeof(T),
                "exprCast must be specialised for this type");
}

template <> inline StringLiteral *exprCast<StringLiteral *>(Expr &expr) {
  return exprCastImpl<StringLiteral *, ExprKind::StringLiteral>(expr);
}

template <> inline NumberLiteral *exprCast<NumberLiteral *>(Expr &expr) {
  return exprCastImpl<NumberLiteral *, ExprKind::NumberLiteral>(expr);
}

template <> inline VarRef *exprCast<VarRef *>(Expr &expr) {
  return exprCastImpl<VarRef *, ExprKind::VarRef>(expr);
}

template <> inline BinaryOp *exprCast<BinaryOp *>(Expr &expr) {
  return exprCastImpl<BinaryOp *, ExprKind::BinaryOp>(expr);
}

template <> inline Call *exprCast<Call *>(Expr &expr) {
  return exprCastImpl<Call *, ExprKind::Call>(expr);
}

template <> inline MemberRef *exprCast<MemberRef *>(Expr &expr) {
  return exprCastImpl<MemberRef *, ExprKind::MemberRef>(expr);
}

template <typename T, StatementKind kind>
inline T statementCastImpl(Statement &ast) {
  if (ast.getKind() == kind)
    return static_cast<T>(&ast);
  return nullptr;
}

template <typename T> inline T statementCast(Statement &) {
  static_assert(sizeof(T) != sizeof(T),
                "statementCast must be specialised for this type");
}

template <>
inline Assignment *statementCast<Assignment *>(Statement &statement) {
  return statementCastImpl<Assignment *, StatementKind::Assignment>(statement);
}

template <> inline Compound *statementCast<Compound *>(Statement &statement) {
  return statementCastImpl<Compound *, StatementKind::Compound>(statement);
}

template <> inline If *statementCast<If *>(Statement &statement) {
  return statementCastImpl<If *, StatementKind::If>(statement);
}

template <> inline Case *statementCast<Case *>(Statement &statement) {
  return statementCastImpl<Case *, StatementKind::Case>(statement);
}

template <> inline While *statementCast<While *>(Statement &statement) {
  return statementCastImpl<While *, StatementKind::While>(statement);
}

template <>
inline CallStatement *statementCast<CallStatement *>(Statement &statement) {
  return statementCastImpl<CallStatement *, StatementKind::Call>(statement);
}

template <> inline For *statementCast<For *>(Statement &statement) {
  return statementCastImpl<For *, StatementKind::For>(statement);
}

} // namespace descartes
