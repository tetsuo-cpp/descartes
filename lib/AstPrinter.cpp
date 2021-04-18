#include "AstPrinter.h"

#include <cassert>
#include <iostream>

namespace descartes {

using json = nlohmann::json;

static size_t indentWidth = 4;

void AstPrinter::printBlock(Block &block) {
  const auto obj = convertBlock(block);
  std::cout << obj.dump(indentWidth) << "\n";
}

json AstPrinter::convertExpr(Expr &expr) {
  if (auto *stringLiteral = exprCast<StringLiteral *>(expr))
    return convertStringLiteral(*stringLiteral);
  else if (auto *numberLiteral = exprCast<NumberLiteral *>(expr))
    return convertNumberLiteral(*numberLiteral);
  else if (auto *varRef = exprCast<VarRef *>(expr))
    return convertVarRef(*varRef);
  else
    assert(!"Unknown node in AstPrinter.");
}

json AstPrinter::convertBlock(Block &block) {
  json blockObj;
  blockObj["Type"] = "Block";
  json labels = json::array();
  for (const auto &label : block.labelDecls)
    labels.emplace_back(label.getName());
  blockObj["Labels"] = labels;
  json constDefs = json::array();
  for (auto &constDef : block.constDefs)
    constDefs.emplace_back(convertConstDef(constDef));
  blockObj["ConstDefs"] = constDefs;
  json typeDefs = convertTypeDefs(block.typeDefs);
  blockObj["TypeDefs"] = typeDefs;
  json varDecls = json::array();
  for (auto &varDecl : block.varDecls)
    varDecls.emplace_back(convertVarDecl(varDecl));
  blockObj["VarDecls"] = varDecls;
  json functions = json::array();
  for (auto &function : block.functions)
    functions.emplace_back(convertFunction(*function));
  blockObj["Functions"] = functions;
  return blockObj;
}

json AstPrinter::convertConstDef(ConstDef &constDef) {
  json constDefObj;
  constDefObj["Type"] = "ConstDef";
  constDefObj["Identifier"] = constDef.identifier.getName();
  constDefObj["ConstExpr"] = convertExpr(*constDef.constExpr);
  return constDefObj;
}

json AstPrinter::convertTypeDefs(TypeDefs &typeDefs) {
  json typeDefsObj = json::array();
  for (const auto &typeDef : typeDefs) {
    json typeDefObj;
    typeDefObj["Type"] = "TypeDef";
    typeDefObj["Identifier"] = typeDef.first.getName();
    typeDefsObj.emplace_back(typeDefObj);
  }
  return typeDefsObj;
}

json AstPrinter::convertVarDecl(VarDecl &varDecl) {
  // TODO: Print type specifiers.
  json varDeclObj;
  varDeclObj["Type"] = "VarDecl";
  varDeclObj["Identifier"] = varDecl.identifier.getName();
  return varDeclObj;
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

json AstPrinter::convertVarRef(VarRef &varRef) {
  json varRefObj;
  varRefObj["Type"] = "VarRef";
  varRefObj["Identifier"] = varRef.identifier.getName();
  return varRefObj;
}

json AstPrinter::convertFunction(Function &function) {
  json functionObj;
  functionObj["Type"] = "Function";
  auto args = json::array();
  for (const auto &arg : function.args) {
    json argObj;
    argObj["Name"] = arg.first.getName();
    argObj["Type"] = arg.second.getName();
    args.emplace_back(argObj);
  }
  functionObj["Args"] = args;
  functionObj["Block"] = convertBlock(function.block);
  return functionObj;
}

} // namespace descartes
