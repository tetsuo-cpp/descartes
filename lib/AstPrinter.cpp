#include "AstPrinter.h"

#include <iostream>

namespace descartes {

static size_t indentWidth = 4;

void AstPrinter::printAst(IAst &ast) {
  indent();
  if (auto *block = astCast<Block *>(ast))
    printBlock(*block);
  else if (auto *constDef = astCast<ConstDef *>(ast))
    printConstDef(*constDef);
  else if (auto *stringLiteral = astCast<StringLiteral *>(ast))
    printStringLiteral(*stringLiteral);
  else if (auto *numberLiteral = astCast<NumberLiteral *>(ast))
    printNumberLiteral(*numberLiteral);
  else if (auto *varRef = astCast<VariableRef *>(ast))
    printVarRef(*varRef);
  else
    assert(!"Unknown node in AstPrinter.");
  unindent();
}

void AstPrinter::indent() { padding.append(indentWidth, ' '); }

void AstPrinter::unindent() {
  for (size_t i = 0; i < indentWidth; ++i)
    padding.pop_back();
}

void AstPrinter::printBlock(Block &block) {
  std::cout << padding << "BLOCK\n";
  std::cout << padding << "labels: ";
  for (const auto &label : block.labelDecl)
    std::cout << label << ", ";
  std::cout << "\n";
  for (auto &constDef : block.constDef)
    printAst(*constDef);
}

void AstPrinter::printConstDef(ConstDef &constDef) {
  std::cout << padding << "CONST_DEF\n";
  std::cout << padding << "identifier: " << constDef.identifier << "\n";
  std::cout << padding << "const_expr:\n";
  printAst(*constDef.constExpr);
}

void AstPrinter::printStringLiteral(StringLiteral &stringLiteral) {
  std::cout << padding << "STRING_LITERAL\n";
  std::cout << padding << "val: " << stringLiteral.val << "\n";
}

void AstPrinter::printNumberLiteral(NumberLiteral &numberLiteral) {
  std::cout << padding << "NUMBER_LITERAL\n";
  std::cout << padding << "val: " << numberLiteral.val << "\n";
}

void AstPrinter::printVarRef(VariableRef &varRef) {
  std::cout << padding << "VAR_REF\n";
  std::cout << padding << "identifier: " << varRef.identifier << "\n";
}

} // namespace descartes
