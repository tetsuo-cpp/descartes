#pragma once

#include <Ast.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace descartes {

class AstPrinter {
public:
  virtual ~AstPrinter() = default;
  void printAst(IAst &program);

private:
  json convertNode(IAst &ast);
  json convertBlock(Block &block);
  json convertConstDef(ConstDef &constDef);
  json convertStringLiteral(StringLiteral &stringLiteral);
  json convertNumberLiteral(NumberLiteral &numberLiteral);
  json convertVarRef(VariableRef &varRef);
};

} // namespace descartes
