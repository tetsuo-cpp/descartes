#pragma once

#include <Ast.h>

#include <nlohmann/json.hpp>

namespace descartes {

class AstPrinter {
public:
  virtual ~AstPrinter() = default;
  void printBlock(Block &block);

private:
  using json = nlohmann::json;
  json convertBlock(Block &block);
  json convertConstDef(ConstDef &constDef);
  json convertTypeDefs(TypeDefs &typeDefs);
  json convertVarDecl(VarDecl &varDecl);
  json convertFunction(Function &function);
  json convertStatement(Statement &statement);
  json convertAssignment(Statement &statement);
  json convertCompound(Statement &statement);
  json convertIf(Statement &statement);
  json convertWhile(Statement &statement);
  json convertFor(Statement &statement);
  json convertCallStatement(Statement &statement);
  json convertExpr(Expr &expr);
  json convertStringLiteral(Expr &expr);
  json convertNumberLiteral(Expr &expr);
  json convertVarRef(Expr &expr);
  json convertBinaryOp(Expr &expr);
  json convertCall(Expr &expr);
  json convertMemberRef(Expr &expr);
};

} // namespace descartes
