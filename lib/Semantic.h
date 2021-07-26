#pragma once

#include <Environment.h>
#include <Interfaces.h>
#include <SymbolTable.h>

namespace descartes {

class Semantic {
public:
  explicit Semantic(const SymbolTable &symbols);
  virtual ~Semantic() = default;
  void analyse(Block &program);

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
  const Type *analyseExpr(Expr &expr);
  const Type *analyseStringLiteral(Expr &expr);
  const Type *analyseNumberLiteral(Expr &expr);
  const Type *analyseVarRef(Expr &expr);
  const Type *analyseBinaryOp(Expr &expr);
  const Type *analyseCall(Expr &expr);
  const Type *analyseMemberRef(Expr &expr);
  bool isCompatibleType(const Type *lhs, const Type *rhs) const;
  const SymbolTable &symbols;
  Environment env;
};

} // namespace descartes
