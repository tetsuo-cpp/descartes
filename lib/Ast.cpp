#include "Ast.h"

#include <cassert>

namespace descartes {

Symbol::Symbol(int id) : id(id), value(nullptr) {}

const std::string &Symbol::getName() const {
  assert(value);
  return *value;
}

bool Symbol::operator==(const Symbol &other) const { return id == other.id; }

ConstDef::ConstDef(Symbol identifier, ExprPtr constExpr)
    : identifier(identifier), constExpr(std::move(constExpr)) {}

VarDecl::VarDecl(Symbol identifier, Symbol type)
    : identifier(identifier), type(type) {}

Block::Block(std::vector<Symbol> &&labelDecls,
             std::vector<ConstDef> &&constDefs, TypeDefs &&typeDefs,
             std::vector<VarDecl> &&varDecls,
             std::vector<std::unique_ptr<Function>> functions,
             StatementPtr statements)
    : labelDecls(std::move(labelDecls)), constDefs(std::move(constDefs)),
      typeDefs(std::move(typeDefs)), varDecls(std::move(varDecls)),
      functions(std::move(functions)), statements(std::move(statements)) {}

Function::Function(Symbol name, std::vector<FunctionArg> &&args, Block &&block,
                   std::optional<Symbol> returnType)
    : name(name), args(std::move(args)), block(std::move(block)),
      returnType(returnType) {}

StringLiteral::StringLiteral(std::string &&val) : val(std::move(val)) {}

ExprKind StringLiteral::getKind() const { return ExprKind::StringLiteral; }

NumberLiteral::NumberLiteral(int val) : val(val) {}

ExprKind NumberLiteral::getKind() const { return ExprKind::NumberLiteral; }

VarRef::VarRef(Symbol identifier) : identifier(identifier) {}

ExprKind VarRef::getKind() const { return ExprKind::VarRef; }

MemberRef::MemberRef(ExprPtr expr, Symbol identifier)
    : expr(std::move(expr)), identifier(identifier) {}

ExprKind MemberRef::getKind() const { return ExprKind::MemberRef; }

const char *binaryOpKindToString(BinaryOpKind kind) {
  switch (kind) {
  case BinaryOpKind::Add:
    return "Add";
  case BinaryOpKind::Subtract:
    return "Subtract";
  case BinaryOpKind::Multiply:
    return "Multiply";
  case BinaryOpKind::Divide:
    return "Divide";
  case BinaryOpKind::Equal:
    return "Equal";
  case BinaryOpKind::LessThan:
    return "LessThan";
  case BinaryOpKind::GreaterThan:
    return "GreaterThan";
  case BinaryOpKind::NotEqual:
    return "NotEqual";
  case BinaryOpKind::LessThanEqual:
    return "LessThanEqual";
  case BinaryOpKind::GreaterThanEqual:
    return "GreaterThanEqual";
  }
  return "";
}

BinaryOp::BinaryOp(BinaryOpKind kind, ExprPtr lhs, ExprPtr rhs)
    : kind(kind), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

ExprKind BinaryOp::getKind() const { return ExprKind::BinaryOp; }

Call::Call(Symbol functionName, std::vector<ExprPtr> args)
    : functionName(functionName), args(std::move(args)) {}

ExprKind Call::getKind() const { return ExprKind::Call; }

Assignment::Assignment(ExprPtr lhs, ExprPtr rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

StatementKind Assignment::getKind() const { return StatementKind::Assignment; }

Compound::Compound(std::vector<StatementPtr> body) : body(std::move(body)) {}

StatementKind Compound::getKind() const { return StatementKind::Compound; }

If::If(ExprPtr cond, StatementPtr thenStatement, StatementPtr elseStatement)
    : cond(std::move(cond)), thenStatement(std::move(thenStatement)),
      elseStatement(std::move(elseStatement)) {}

StatementKind If::getKind() const { return StatementKind::If; }

StatementKind Case::getKind() const { return StatementKind::Case; }

Repeat::Repeat(ExprPtr untilCond, std::vector<StatementPtr> body)
    : untilCond(std::move(untilCond)), body(std::move(body)) {}

StatementKind Repeat::getKind() const { return StatementKind::Repeat; }

While::While(ExprPtr cond, StatementPtr body)
    : cond(std::move(cond)), body(std::move(body)) {}

StatementKind While::getKind() const { return StatementKind::While; }

For::For(Symbol controlIdentifier, ExprPtr begin, ExprPtr end, bool to,
         StatementPtr body)
    : controlIdentifier(controlIdentifier), begin(std::move(begin)),
      end(std::move(end)), to(to), body(std::move(body)) {}

StatementKind For::getKind() const { return StatementKind::For; }

With::With(std::vector<Symbol> &&recordIdentifiers, StatementPtr body)
    : recordIdentifiers(std::move(recordIdentifiers)), body(std::move(body)) {}

StatementKind With::getKind() const { return StatementKind::With; }

CallStatement::CallStatement(ExprPtr call) : call(std::move(call)) {}

StatementKind CallStatement::getKind() const { return StatementKind::Call; }

} // namespace descartes
