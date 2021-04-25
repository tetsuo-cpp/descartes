#pragma once

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
  void analyseTypeDefs(const TypeDefs &typeDefs);
  void analyseVarDecls(const std::vector<VarDecl> &varDecls);
  void
  analyseFunctions(const std::vector<std::unique_ptr<Function>> &functions);
  void analyseStatements(Statement &statement);
  void analyseStatement(Statement &statement);
  void analyseAssignment(Statement &statement);
  void analyseCompound(Statement &statement);
  void analyseIf(Statement &statement);
  void analyseCase(Statement &statement);
  void analyseWhile(Statement &statement);
  void analyseCallStatement(Statement &statement);
  const Type *analyseExpr(Expr &expr);
  const SymbolTable &symbols;
};

} // namespace descartes
