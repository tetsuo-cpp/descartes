#include "AstPrinter.h"

#include <cassert>
#include <iostream>

namespace descartes {

static size_t indentWidth = 4;

void AstPrinter::printAst(IAst &program) {
  const auto obj = convertNode(program);
  std::cout << obj.dump(indentWidth) << "\n";
}

json AstPrinter::convertNode(IAst &ast) {
  if (auto *block = astCast<Block *>(ast))
    return convertBlock(*block);
  else if (auto *constDef = astCast<ConstDef *>(ast))
    return convertConstDef(*constDef);
  else if (auto *stringLiteral = astCast<StringLiteral *>(ast))
    return convertStringLiteral(*stringLiteral);
  else if (auto *numberLiteral = astCast<NumberLiteral *>(ast))
    return convertNumberLiteral(*numberLiteral);
  else if (auto *varRef = astCast<VariableRef *>(ast))
    return convertVarRef(*varRef);
  else
    assert(!"Unknown node in AstPrinter.");
}

json AstPrinter::convertBlock(Block &block) {
  json blockObj;
  blockObj["Type"] = "Block";
  json labels = json::array();
  for (const auto &label : block.labelDecl)
    labels.emplace_back(label);
  blockObj["Labels"] = labels;
  json constDefs = json::array();
  for (auto &constDef : block.constDef)
    constDefs.emplace_back(convertNode(*constDef));
  blockObj["ConstDefs"] = constDefs;
  return blockObj;
}

json AstPrinter::convertConstDef(ConstDef &constDef) {
  json constDefObj;
  constDefObj["Type"] = "ConstDef";
  constDefObj["Identifier"] = constDef.identifier;
  constDefObj["ConstExpr"] = convertNode(*constDef.constExpr);
  return constDefObj;
}

json AstPrinter::convertStringLiteral(StringLiteral &stringLiteral) {
  json stringLiteralObj;
  stringLiteralObj["Type"] = "StringLiteral";
  stringLiteralObj["Val"] = stringLiteral.val;
  return stringLiteralObj;
}

json AstPrinter::convertNumberLiteral(NumberLiteral &numberLiteral) {
  json numberLiteralObj;
  numberLiteralObj["Type"] = "NumberLiteral";
  numberLiteralObj["Val"] = numberLiteral.val;
  return numberLiteralObj;
}

json AstPrinter::convertVarRef(VariableRef &varRef) {
  json varRefObj;
  varRefObj["Type"] = "VarRef";
  varRefObj["Identifier"] = varRef.identifier;
  return varRefObj;
}

} // namespace descartes
