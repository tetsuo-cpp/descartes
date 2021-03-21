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
  json convertExpr(Expr &expr);
  json convertBlock(Block &block);
  json convertConstDef(ConstDef &constDef);
  json convertTypeDefs(TypeDefs &typeDefs);
  json convertVarDecl(VarDecl &varDecl);
  json convertStringLiteral(StringLiteral &stringLiteral);
  json convertNumberLiteral(NumberLiteral &numberLiteral);
  json convertVarRef(VarRef &varRef);
  json convertFunction(Function &function);
};

} // namespace descartes
