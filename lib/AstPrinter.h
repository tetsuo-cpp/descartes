#pragma once

#include <Ast.h>

namespace descartes {

class AstPrinter {
public:
  virtual ~AstPrinter() = default;
  void printAst(IAst &program);

private:
  void indent();
  void unindent();
  void printBlock(Block &block);
  void printConstDef(ConstDef &constDef);
  void printStringLiteral(StringLiteral &stringLiteral);
  void printNumberLiteral(NumberLiteral &numberLiteral);
  void printVarRef(VariableRef &varRef);
  std::string padding;
};

} // namespace descartes
