#include "Translate.h"

namespace descartes {

namespace {

ir::ArithOpKind binOpKindToArithOpKind(BinaryOpKind kind) {
  switch (kind) {
  case BinaryOpKind::Add:
    return ir::ArithOpKind::Add;
  case BinaryOpKind::Subtract:
    return ir::ArithOpKind::Subtract;
  case BinaryOpKind::Multiply:
    return ir::ArithOpKind::Multiply;
  case BinaryOpKind::Divide:
    return ir::ArithOpKind::Divide;
  default:
    assert(false);
  }
}

ir::RelOpKind binOpKindToRelOpKind(BinaryOpKind kind) {
  switch (kind) {
  case BinaryOpKind::Equal:
    return ir::RelOpKind::Equal;
  case BinaryOpKind::NotEqual:
    return ir::RelOpKind::NotEqual;
  case BinaryOpKind::LessThan:
    return ir::RelOpKind::LessThan;
  case BinaryOpKind::LessThanEqual:
    return ir::RelOpKind::LessThanEqual;
  case BinaryOpKind::GreaterThan:
    return ir::RelOpKind::GreaterThan;
  case BinaryOpKind::GreaterThanEqual:
    return ir::RelOpKind::GreaterThanEqual;
  default:
    assert(false);
  }
}

} // namespace

Translate::Translate(SymbolTable &symbols) : symbols(symbols), labelCount(0) {}

ir::ExprPtr Translate::makeName(const StringLiteral &stringLiteral) const {
  return std::make_unique<ir::Name>(stringLiteral.val);
}

ir::ExprPtr Translate::makeConst(const NumberLiteral &numberLiteral) const {
  return std::make_unique<ir::Const>(numberLiteral.val);
}

ir::ExprPtr Translate::makeArithOp(BinaryOpKind kind, ir::ExprPtr lhs,
                                   ir::ExprPtr rhs) const {
  const ir::ArithOpKind k = binOpKindToArithOpKind(kind);
  return std::make_unique<ir::ArithOp>(k, std::move(lhs), std::move(rhs));
}

ir::ExprPtr Translate::makeCondJump(BinaryOpKind kind, ir::ExprPtr lhs,
                                    ir::ExprPtr rhs) {
  const Symbol thenLabel = makeLabel(), elseLabel = makeLabel();
  const ir::RelOpKind k = binOpKindToRelOpKind(kind);
  auto condJump = std::make_unique<ir::CondJump>(
      k, std::move(lhs), std::move(rhs), thenLabel, elseLabel);
  auto condExpr = std::make_unique<ir::CondExpr>(std::move(condJump));
  return condExpr;
}

void Translate::pushFrag(const ir::Level &level, StatementPtr body) {
  frags.emplace_back(level, std::move(body));
}

const std::vector<ir::Fragment> &Translate::getFrags() const { return frags; }

void Translate::enterLevel(Symbol name) {
  levels.push_back(std::make_unique<ir::Level>(name));
}

void Translate::exitLevel() { levels.pop_back(); }

const ir::Level *Translate::getCurrentLevel() const {
  return levels.back().get();
}

// TODO: Make a label type to ensure that they're not exchangeable with symbols.
Symbol Translate::makeLabel() {
  const std::string labelName = "L" + std::to_string(labelCount++);
  return symbols.make(labelName);
}

} // namespace descartes
