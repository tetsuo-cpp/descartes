#pragma once

#include "Ir.h"

namespace descartes {

class Translate {
public:
  explicit Translate(SymbolTable &symbols);
  virtual ~Translate() = default;
  ir::StatementPtr makeMove(ir::ExprPtr &&lhs, ir::ExprPtr &&rhs);
  ir::StatementPtr makeSequence(std::vector<ir::StatementPtr> &&body);
  ir::StatementPtr makeIf(ir::ExprPtr &&condExpr,
                          ir::StatementPtr &&thenStatement,
                          ir::StatementPtr &&elseStatement);
  ir::StatementPtr makeWhile(ir::ExprPtr &&condExpr, ir::StatementPtr &&body);
  ir::StatementPtr makeCallStatement(ir::ExprPtr &&callExpr) const;
  ir::ExprPtr makeName(const StringLiteral &stringLiteral) const;
  ir::ExprPtr makeConst(const NumberLiteral &numberLiteral) const;
  ir::ExprPtr makeVarRef(const VarRef &varRef) const;
  ir::ExprPtr makeArithOp(BinaryOpKind kind, ir::ExprPtr lhs,
                          ir::ExprPtr rhs) const;
  ir::ExprPtr makeCondJump(BinaryOpKind kind, ir::ExprPtr lhs, ir::ExprPtr rhs);
  void pushFrag(const ir::Level &level, StatementPtr body);
  const std::vector<ir::Fragment> &getFrags() const;
  void enterLevel(Symbol name);
  void exitLevel();
  ir::Level *getCurrentLevel();

private:
  Symbol makeLabel();
  SymbolTable &symbols;
  std::vector<ir::Fragment> frags;
  std::vector<std::unique_ptr<ir::Level>> levels;
  int labelCount;
};

} // namespace descartes
