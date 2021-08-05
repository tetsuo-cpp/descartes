#pragma once

#include "Ir.h"

namespace descartes {

class Translate {
public:
  explicit Translate(SymbolTable &symbols);
  virtual ~Translate() = default;
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
  const ir::Level *getCurrentLevel() const;

private:
  Symbol makeLabel();
  SymbolTable &symbols;
  std::vector<ir::Fragment> frags;
  std::vector<std::unique_ptr<ir::Level>> levels;
  int labelCount;
};

} // namespace descartes
