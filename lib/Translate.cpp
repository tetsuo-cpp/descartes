#include "Translate.h"

#include <cassert>

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
    throw SemanticError("Invalid ArithOp kind");
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
    throw SemanticError("Invalid RelOp kind");
  }
}

} // namespace

Translate::Translate(SymbolTable &symbols) : symbols(symbols), labelCount(0) {}

ir::StatementPtr Translate::makeMove(ir::ExprPtr &&lhs, ir::ExprPtr &&rhs) {
  return std::make_unique<ir::Move>(std::move(lhs), std::move(rhs));
}

ir::StatementPtr Translate::makeSequence(std::vector<ir::StatementPtr> &&body) {
  return std::make_unique<ir::Sequence>(std::move(body));
}

ir::StatementPtr Translate::makeIf(ir::ExprPtr &&condExpr,
                                   ir::StatementPtr &&thenStatement,
                                   ir::StatementPtr &&elseStatement) {
  // TODO: Refactor to avoid repetition.
  // This means we've got a nested conditional jump here.
  if (condExpr->getKind() == ir::ExprKind::CondExpr) {
    auto *condExprPtr = static_cast<ir::CondExpr *>(condExpr.get());
    auto *condJumpPtr =
        static_cast<ir::CondJump *>(condExprPtr->condJump.get());
    assert(condJumpPtr);
    // Save the then and else labels.
    std::vector<ir::StatementPtr> seq;
    seq.push_back(std::move(condExprPtr->condJump));
    seq.push_back(std::make_unique<ir::Label>(condJumpPtr->thenLabel));
    seq.push_back(std::move(thenStatement));
    // Else statement is optional.
    if (elseStatement) {
      seq.push_back(std::make_unique<ir::Label>(condJumpPtr->elseLabel));
      seq.push_back(std::move(elseStatement));
    }
    return makeSequence(std::move(seq));
  }
  // Otherwise it's something else that resolves to a boolean such as a variable
  // or function call.
  Symbol thenLabel = makeLabel(), elseLabel = makeLabel();
  auto cond = std::make_unique<ir::CondJump>(
      ir::RelOpKind::Equal, std::move(condExpr), std::make_unique<ir::Const>(1),
      thenLabel, elseLabel);
  std::vector<ir::StatementPtr> seq;
  seq.push_back(std::move(cond));
  seq.push_back(std::make_unique<ir::Label>(thenLabel));
  seq.push_back(std::move(thenStatement));
  if (elseStatement) {
    // TODO: Make labels optional for conditionals in the IR.
    //
    // We need a proper way of saying that we don't intend to jump to an `else`
    // block.
    seq.push_back(std::make_unique<ir::Label>(elseLabel));
    seq.push_back(std::move(elseStatement));
  }
  return makeSequence(std::move(seq));
}

ir::StatementPtr Translate::makeWhile(ir::ExprPtr &&condExpr,
                                      ir::StatementPtr &&body) {
  if (condExpr->getKind() == ir::ExprKind::CondExpr) {
    auto *condExprPtr = static_cast<ir::CondExpr *>(condExpr.get());
    auto *condJumpPtr =
        static_cast<ir::CondJump *>(condExprPtr->condJump.get());
    assert(condJumpPtr);
    const Symbol condLabel = makeLabel();
    std::vector<ir::StatementPtr> seq;
    seq.push_back(std::make_unique<ir::Label>(condLabel));
    seq.push_back(std::move(condExprPtr->condJump));
    seq.push_back(std::make_unique<ir::Label>(condJumpPtr->thenLabel));
    seq.push_back(std::move(body));
    seq.push_back(std::make_unique<ir::Jump>(condLabel));
    seq.push_back(std::make_unique<ir::Label>(condJumpPtr->elseLabel));
    return makeSequence(std::move(seq));
  }
  // Otherwise it's something else that resolves to a boolean such as a variable
  // function or call.
  const Symbol thenLabel = makeLabel(), elseLabel = makeLabel(),
               condLabel = makeLabel();
  auto cond = std::make_unique<ir::CondJump>(
      ir::RelOpKind::Equal, std::move(condExpr), std::make_unique<ir::Const>(1),
      thenLabel, elseLabel);
  std::vector<ir::StatementPtr> seq;
  seq.push_back(std::make_unique<ir::Label>(condLabel));
  seq.push_back(std::move(cond));
  seq.push_back(std::make_unique<ir::Label>(cond->thenLabel));
  seq.push_back(std::move(body));
  seq.push_back(std::make_unique<ir::Jump>(condLabel));
  seq.push_back(std::make_unique<ir::Label>(cond->elseLabel));
  return makeSequence(std::move(seq));
}

ir::StatementPtr Translate::makeCallStatement(ir::ExprPtr &&callExpr) const {
  return std::make_unique<ir::CallStatement>(std::move(callExpr));
}

ir::ExprPtr Translate::makeName(const StringLiteral &stringLiteral) const {
  return std::make_unique<ir::Name>(stringLiteral.val);
}

ir::ExprPtr Translate::makeConst(const NumberLiteral &numberLiteral) const {
  return std::make_unique<ir::Const>(numberLiteral.val);
}

ir::ExprPtr Translate::makeVarRef(ir::Access access) const {
  // We should be checking from the current frame onwards.
  ir::ExprPtr frameAddr = getCurrentFramePointer();
  for (auto levelIt = levels.rbegin(); levelIt != levels.rend(); ++levelIt) {
    const auto &currentLevel = *levelIt;
    if (currentLevel.get() == access.level) {
      // The memory address is the offset from the frame pointer.
      auto memAddress = std::make_unique<ir::ArithOp>(
          ir::ArithOpKind::Add, std::move(frameAddr),
          std::make_unique<ir::Const>(access.offset));
      return std::make_unique<ir::Mem>(std::move(memAddress));
    } else {
      // Since it's not in the current frame, we need to read the first arg
      // (static link) and get the address of the parent frame.
      const ir::Access staticLink = currentLevel->locals.front();
      auto frameMem = std::make_unique<ir::ArithOp>(
          ir::ArithOpKind::Add, std::move(frameAddr),
          std::make_unique<ir::Const>(staticLink.offset));
      frameAddr = std::make_unique<ir::Mem>(std::move(frameMem));
    }
  }
  throw SemanticError("Could not find frame owning access");
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

ir::Level *Translate::getCurrentLevel() { return levels.back().get(); }

// TODO: Make a label type to ensure that they're not exchangeable with symbols.
Symbol Translate::makeLabel() {
  const std::string labelName = "L" + std::to_string(labelCount++);
  return symbols.make(labelName);
}

ir::ExprPtr Translate::getCurrentFramePointer() const {
  // TODO: Get current frame pointer for each arch.
  return nullptr;
}

} // namespace descartes
