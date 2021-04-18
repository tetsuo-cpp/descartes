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
  json varDeclObj;
  varDeclObj["Type"] = "VarDecl";
  varDeclObj["Identifier"] = varDecl.identifier.getName();
  varDeclObj["Type"] = varDecl.type.getName();
  return varDeclObj;
}

json AstPrinter::convertFunction(Function &function) {
  json functionObj;
  functionObj["Type"] = "Function";
  auto args = json::array();
  for (const auto &arg : function.args) {
    json argObj;
    argObj["Name"] = arg.identifier.getName();
    argObj["Type"] = arg.type.getName();
    argObj["IsConst"] = arg.isConst;
    args.emplace_back(argObj);
  }
  functionObj["Args"] = args;
  functionObj["Block"] = convertBlock(function.block);
  return functionObj;
}

json AstPrinter::convertStatement(Statement &statement) {
  switch (statement.getKind()) {
  case StatementKind::Assignment:
    return convertAssignment(statement);
  case StatementKind::Compound:
    return convertCompound(statement);
  case StatementKind::If:
    return convertIf(statement);
  case StatementKind::While:
    return convertWhile(statement);
  case StatementKind::Call:
    return convertCallStatement(statement);
  default:
    assert(!"Unsupported statement type");
  }
}

json AstPrinter::convertAssignment(Statement &statement) {
  auto *assignment = statementCast<Assignment *>(statement);
  assert(assignment);
  json assignmentObj;
  assignmentObj["Left"] = convertExpr(*assignment->lhs);
  assignmentObj["Right"] = convertExpr(*assignment->rhs);
  return assignmentObj;
}

json AstPrinter::convertCompound(Statement &statement) {
  auto *compound = statementCast<Compound *>(statement);
  assert(compound);
  json compoundObj = json::array();
  for (const auto &s : compound->body)
    compoundObj.emplace_back(convertStatement(*s));
  return compoundObj;
}

json AstPrinter::convertIf(Statement &statement) {
  auto *ifStatement = statementCast<If *>(statement);
  assert(ifStatement);
  json ifObj;
  ifObj["Type"] = "If";
  ifObj["Cond"] = convertExpr(*ifStatement->cond);
  ifObj["Then"] = convertStatement(*ifStatement->thenStatement);
  if (ifStatement->elseStatement)
    ifObj["Else"] = convertStatement(*ifStatement->elseStatement);
  return ifObj;
}

json AstPrinter::convertWhile(Statement &statement) {
  auto *whileStatement = statementCast<While *>(statement);
  assert(whileStatement);
  json whileObj;
  whileObj["Type"] = "While";
  whileObj["Cond"] = convertExpr(*whileStatement->cond);
  whileObj["Body"] = convertStatement(*whileStatement->body);
  return whileObj;
}

json AstPrinter::convertFor(Statement &statement) {
  auto *forStatement = statementCast<For *>(statement);
  assert(forStatement);
  json forObj;
  forObj["Type"] = "For";
  forObj["Begin"] = convertExpr(*forStatement->begin);
  forObj["End"] = convertExpr(*forStatement->end);
  forObj["To"] = forStatement->to;
  forObj["Body"] = convertStatement(*forStatement->body);
  return forObj;
}

json AstPrinter::convertCallStatement(Statement &statement) {
  auto *call = statementCast<CallStatement *>(statement);
  assert(call);
  json callObj;
  callObj["Type"] = "CallStatement";
  callObj["Call"] = convertExpr(*call->call);
  return callObj;
}

json AstPrinter::convertExpr(Expr &expr) {
  switch (expr.getKind()) {
  case ExprKind::StringLiteral:
    return convertStringLiteral(expr);
  case ExprKind::NumberLiteral:
    return convertNumberLiteral(expr);
  case ExprKind::VarRef:
    return convertVarRef(expr);
  case ExprKind::BinaryOp:
    return convertBinaryOp(expr);
  case ExprKind::Call:
    return convertCall(expr);
  case ExprKind::MemberRef:
    return convertMemberRef(expr);
  }
  return json::object();
}

json AstPrinter::convertStringLiteral(Expr &expr) {
  auto *stringLiteral = exprCast<StringLiteral *>(expr);
  assert(stringLiteral);
  json stringLiteralObj;
  stringLiteralObj["Type"] = "StringLiteral";
  stringLiteralObj["Val"] = stringLiteral->val;
  return stringLiteralObj;
}

json AstPrinter::convertNumberLiteral(Expr &expr) {
  auto *numberLiteral = exprCast<NumberLiteral *>(expr);
  assert(numberLiteral);
  json numberLiteralObj;
  numberLiteralObj["Type"] = "NumberLiteral";
  numberLiteralObj["Val"] = numberLiteral->val;
  return numberLiteralObj;
}

json AstPrinter::convertVarRef(Expr &expr) {
  auto *varRef = exprCast<VarRef *>(expr);
  assert(varRef);
  json varRefObj;
  varRefObj["Type"] = "VarRef";
  varRefObj["Identifier"] = varRef->identifier.getName();
  return varRefObj;
}

json AstPrinter::convertBinaryOp(Expr &expr) {
  auto *binaryOp = exprCast<BinaryOp *>(expr);
  assert(binaryOp);
  json binaryOpObj;
  binaryOpObj["Type"] = "BinaryOp";
  binaryOpObj["Left"] = convertExpr(*binaryOp->lhs);
  binaryOpObj["Right"] = convertExpr(*binaryOp->rhs);
  binaryOpObj["Operator"] = binaryOpKindToString(binaryOp->kind);
  return binaryOpObj;
}

json AstPrinter::convertCall(Expr &expr) {
  auto *call = exprCast<Call *>(expr);
  assert(call);
  json callObj;
  callObj["Type"] = "Call";
  callObj["Name"] = call->functionName.getName();
  json argObjs;
  for (const auto &arg : call->args)
    argObjs.emplace_back(convertExpr(*arg));
  callObj["Args"] = argObjs;
  return callObj;
}

json AstPrinter::convertMemberRef(Expr &expr) {
  auto *memberRef = exprCast<MemberRef *>(expr);
  assert(memberRef);
  json memberRefObj;
  memberRefObj["Type"] = "MemberRef";
  memberRefObj["Expr"] = convertExpr(*memberRef->expr);
  memberRefObj["Identifier"] = memberRef->identifier.getName();
  return memberRefObj;
}

} // namespace descartes
