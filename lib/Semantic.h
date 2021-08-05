#pragma once

#include <Environment.h>
#include <Interfaces.h>
#include <SymbolTable.h>
#include <Translate.h>

namespace descartes {

class Semantic {
public:
  explicit Semantic(const SymbolTable &symbols);
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
  void analyseStatement(Statement &statement);
  void analyseAssignment(Statement &statement);
  void analyseCompound(Statement &statement);
  void analyseIf(Statement &statement);
  void analyseCase(Statement &statement);
  void analyseWhile(Statement &statement);
  void analyseCallStatement(Statement &statement);
  using ExprResult = std::pair<ir::ExprPtr, const Type *>;
  ExprResult analyseExpr(Expr &expr);
  ExprResult analyseStringLiteral(Expr &expr);
  ExprResult analyseNumberLiteral(Expr &expr);
  ExprResult analyseVarRef(Expr &expr);
  ExprResult analyseBinaryOp(Expr &expr);
  ExprResult analyseCall(Expr &expr);
  ExprResult analyseMemberRef(Expr &expr);
  bool isCompatibleType(const Type *lhs, const Type *rhs) const;
  const SymbolTable &symbols;
  Environment env;
  Translate translate;
};

} // namespace descartes
