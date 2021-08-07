#pragma once

#include <Environment.h>
#include <Interfaces.h>
#include <SymbolTable.h>
#include <Translate.h>

namespace descartes {

class Semantic {
public:
  explicit Semantic(SymbolTable &symbols);
  virtual ~Semantic() = default;
  const std::vector<ir::Fragment> &analyse(Block &program);

private:
  void analyseBlock(Block &block);
  void analyseConstDefs(const std::vector<ConstDef> &constDefs);
  void analyseTypeDefs(const std::vector<TypeDef> &typeDefs);
  void analyseVarDecls(const std::vector<VarDecl> &varDecls);
  void
  analyseFunctions(const std::vector<std::unique_ptr<Function>> &functions);
  void analyseBlockStatements(Statement &statement);
  ir::StatementPtr analyseStatement(Statement &statement);
  ir::StatementPtr analyseAssignment(Statement &statement);
  ir::StatementPtr analyseCompound(Statement &statement);
  ir::StatementPtr analyseIf(Statement &statement);
  ir::StatementPtr analyseCase(Statement &statement);
  ir::StatementPtr analyseWhile(Statement &statement);
  ir::StatementPtr analyseCallStatement(Statement &statement);
  using ExprResult = std::pair<ir::ExprPtr, const Type *>;
  ExprResult analyseExpr(Expr &expr);
  ExprResult analyseStringLiteral(Expr &expr);
  ExprResult analyseNumberLiteral(Expr &expr);
  ExprResult analyseVarRef(Expr &expr);
  ExprResult analyseBinaryOp(Expr &expr);
  ExprResult analyseCall(Expr &expr);
  ExprResult analyseMemberRef(Expr &expr);
  bool isCompatibleType(const Type *lhs, const Type *rhs) const;
  SymbolTable &symbols;
  Environment env;
  Translate translate;
};

} // namespace descartes
