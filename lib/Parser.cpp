#include "Parser.h"

#include <Ast.h>

#include <cassert>

namespace descartes {

Parser::Parser(ILexer &lexer) : lexer(lexer), currentToken(TokenKind::Eof) {
  readToken();
}

Block Parser::parse() {
  Block programBlock = parseBlock();
  expectToken(TokenKind::Period);
  return programBlock;
}

void Parser::readToken() { currentToken = lexer.lex(); }

bool Parser::isDone() const { return !static_cast<bool>(currentToken); }

bool Parser::checkToken(TokenKind kind) {
  if (currentToken.kind == kind) {
    readToken();
    return true;
  }
  return false;
}

void Parser::expectToken(TokenKind kind) {
  if (!checkToken(kind)) {
#ifndef NDEBUG
    assert(!"Unexpected token");
#endif
    throw ParserError("Unexpected token");
  }
}

Block Parser::parseBlock() {
  std::vector<Symbol> labelDecls;
  if (currentToken.kind == TokenKind::Label)
    labelDecls = parseLabelDecls();
  std::vector<ConstDef> constDefs;
  if (currentToken.kind == TokenKind::Const)
    constDefs = parseConstDefs();
  TypeDefs typeDefs;
  if (currentToken.kind == TokenKind::Type)
    typeDefs = parseTypeDefs();
  std::vector<VarDecl> varDecls;
  if (currentToken.kind == TokenKind::Var)
    varDecls = parseVarDecls();
  std::vector<std::unique_ptr<Function>> functions;
  if (currentToken.kind == TokenKind::Function ||
      currentToken.kind == TokenKind::Procedure)
    functions = parseFunctions();
  expectToken(TokenKind::Begin);
  auto statements = parseCompoundStatement();
  return Block(std::move(labelDecls), std::move(constDefs), std::move(typeDefs),
               std::move(varDecls), std::move(functions),
               std::move(statements));
}

std::vector<Symbol> Parser::parseLabelDecls() {
  expectToken(TokenKind::Label);
  std::vector<Symbol> labels;
  while (!checkToken(TokenKind::SemiColon)) {
    if (!labels.empty())
      expectToken(TokenKind::Comma);
    const auto labelName = currentToken.val;
    expectToken(TokenKind::Identifier);
    labels.push_back(symbols.make(labelName));
  }
  return labels;
}

std::vector<ConstDef> Parser::parseConstDefs() {
  expectToken(TokenKind::Const);
  std::vector<ConstDef> constDefs;
  // This marks the beginning of a subsequent section in the block. If we see
  // this, then get out.
  while (!isDone() && currentToken.kind != TokenKind::Type &&
         currentToken.kind != TokenKind::Var &&
         currentToken.kind != TokenKind::Function &&
         currentToken.kind != TokenKind::Procedure &&
         currentToken.kind != TokenKind::Begin) {
    const auto identifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Equal);
    auto constExpr = parseConstExpr();
    constDefs.emplace_back(symbols.make(identifier), std::move(constExpr));
    expectToken(TokenKind::SemiColon);
  }
  return constDefs;
}

ExprPtr Parser::parseConstExpr() { return parsePrimaryExpr(); }

TypeDefs Parser::parseTypeDefs() {
  expectToken(TokenKind::Type);
  TypeDefs typeDefs;
  while (!isDone() && currentToken.kind != TokenKind::Var &&
         currentToken.kind != TokenKind::Function &&
         currentToken.kind != TokenKind::Procedure &&
         currentToken.kind != TokenKind::Begin) {
    const auto typeIdentifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Equal);
    auto type = parseType();
    expectToken(TokenKind::SemiColon);
    typeDefs.emplace(symbols.make(typeIdentifier), std::move(type));
  }
  return typeDefs;
}

TypePtr Parser::parseType() {
  const bool isPointer = checkToken(TokenKind::Hat);
  const auto typeString = currentToken.val;
  TypePtr type = nullptr;
  if (checkToken(TokenKind::Identifier)) {
    if (typeString == "integer")
      type = std::make_unique<Integer>();
    else if (typeString == "boolean")
      type = std::make_unique<Boolean>();
    else
      type = std::make_unique<Alias>(symbols.make(typeString));
  } else if (checkToken(TokenKind::OpenParen))
    type = parseEnum();
  else if (checkToken(TokenKind::Record))
    type = parseRecord();
  else
    assert(!"Unknown type spec");
  assert(type);
  type->isPointer = isPointer;
  return type;
}

TypePtr Parser::parseEnum() {
  std::vector<Symbol> enums;
  while (!checkToken(TokenKind::CloseParen)) {
    if (!enums.empty())
      expectToken(TokenKind::Comma);
    auto enumVal = currentToken.val;
    expectToken(TokenKind::Identifier);
    enums.push_back(symbols.make(enumVal));
  }
  return std::make_unique<Enum>(std::move(enums));
}

TypePtr Parser::parseRecord() {
  std::vector<std::pair<Symbol, Symbol>> fields;
  while (!isDone() && currentToken.kind != TokenKind::End) {
    const auto fieldIdentifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Colon);
    const auto typeIdentifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    fields.emplace_back(symbols.make(fieldIdentifier),
                        symbols.make(typeIdentifier));
    if (currentToken.kind != TokenKind::End)
      expectToken(TokenKind::SemiColon);
  }
  expectToken(TokenKind::End);
  return std::make_unique<Record>(std::move(fields));
}

std::vector<VarDecl> Parser::parseVarDecls() {
  expectToken(TokenKind::Var);
  std::vector<VarDecl> varDecls;
  while (!isDone() && currentToken.kind != TokenKind::Function &&
         currentToken.kind != TokenKind::Procedure &&
         currentToken.kind != TokenKind::Begin) {
    auto varIdentifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Colon);
    auto typeIdentifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    varDecls.emplace_back(symbols.make(varIdentifier),
                          symbols.make(typeIdentifier));
    expectToken(TokenKind::SemiColon);
  }
  return varDecls;
}

std::vector<std::unique_ptr<Function>> Parser::parseFunctions() {
  std::vector<std::unique_ptr<Function>> functions;
  while (!isDone() && currentToken.kind != TokenKind::Begin) {
    std::unique_ptr<Function> function;
    if (checkToken(TokenKind::Procedure))
      function = parseProcedure();
    else if (checkToken(TokenKind::Function))
      function = parseFunction();
    else
      throw ParserError("Expected either procedure or function");
    functions.push_back(std::move(function));
  }
  return functions;
}

std::unique_ptr<Function> Parser::parseProcedure() {
  const auto procedureName = currentToken.val;
  expectToken(TokenKind::Identifier);
  auto argsList = parseArgsList();
  expectToken(TokenKind::SemiColon);
  auto functionBlock = parseBlock();
  expectToken(TokenKind::SemiColon);
  // No return type for a procedure.
  return std::make_unique<Function>(
      symbols.make(procedureName), std::move(argsList),
      std::move(functionBlock), std::optional<Symbol>{});
}

std::unique_ptr<Function> Parser::parseFunction() {
  const auto functionName = currentToken.val;
  expectToken(TokenKind::Identifier);
  auto argsList = parseArgsList();
  expectToken(TokenKind::Colon);
  const auto returnType = currentToken.val;
  expectToken(TokenKind::Identifier);
  expectToken(TokenKind::SemiColon);
  auto functionBlock = parseBlock();
  expectToken(TokenKind::SemiColon);
  return std::make_unique<Function>(
      symbols.make(functionName), std::move(argsList), std::move(functionBlock),
      symbols.make(returnType));
}

std::vector<std::pair<Symbol, Symbol>> Parser::parseArgsList() {
  std::vector<std::pair<Symbol, Symbol>> argsList;
  expectToken(TokenKind::OpenParen);
  while (!isDone() && currentToken.kind != TokenKind::CloseParen) {
    // TODO: Support const args.
    if (!argsList.empty())
      expectToken(TokenKind::Comma);
    const auto argName = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Colon);
    const auto argType = currentToken.val;
    expectToken(TokenKind::Identifier);
    argsList.emplace_back(symbols.make(argName), symbols.make(argType));
  }
  expectToken(TokenKind::CloseParen);
  return argsList;
}

StatementPtr Parser::parseStatement() {
  if (checkToken(TokenKind::Begin))
    return parseCompoundStatement();
  else if (checkToken(TokenKind::If))
    return parseIf();
  else if (checkToken(TokenKind::Case))
    return parseCase();
  else if (checkToken(TokenKind::Repeat))
    return parseRepeat();
  else if (checkToken(TokenKind::While))
    return parseWhile();
  else if (checkToken(TokenKind::For))
    return parseFor();
  else if (checkToken(TokenKind::With))
    return parseWith();
  else
    return parseIdentifierStatement();
}

ExprPtr Parser::parseExpr() { return parseEquality(); }

ExprPtr Parser::parseEquality() {
  auto lhs = parseRelational();
  for (;;) {
    const auto tokenKind = currentToken.kind;
    if (checkToken(TokenKind::Equal) || checkToken(TokenKind::NotEqual))
      lhs = std::make_unique<BinaryOp>(tokenKindToBinaryOpKind(tokenKind),
                                       std::move(lhs), parseRelational());
    else
      return lhs;
  }
}

ExprPtr Parser::parseRelational() {
  auto lhs = parseAddition();
  for (;;) {
    const auto tokenKind = currentToken.kind;
    if (checkToken(TokenKind::LessThan) || checkToken(TokenKind::GreaterThan) ||
        checkToken(TokenKind::GreaterThanEqual) ||
        checkToken(TokenKind::LessThanEqual))
      lhs = std::make_unique<BinaryOp>(tokenKindToBinaryOpKind(tokenKind),
                                       std::move(lhs), parseAddition());
    else
      return lhs;
  }
}

ExprPtr Parser::parseAddition() {
  auto lhs = parseMultiplication();
  for (;;) {
    const auto tokenKind = currentToken.kind;
    if (checkToken(TokenKind::Add) || checkToken(TokenKind::Subtract))
      lhs = std::make_unique<BinaryOp>(tokenKindToBinaryOpKind(tokenKind),
                                       std::move(lhs), parseMultiplication());
    else
      return lhs;
  }
}

ExprPtr Parser::parseMultiplication() {
  auto lhs = parsePostfix();
  for (;;) {
    const auto tokenKind = currentToken.kind;
    if (checkToken(TokenKind::Multiply) || checkToken(TokenKind::Divide))
      lhs = std::make_unique<BinaryOp>(tokenKindToBinaryOpKind(tokenKind),
                                       std::move(lhs), parsePostfix());
    else
      return lhs;
  }
}

ExprPtr Parser::parsePostfix() {
  auto expr = parsePrimaryExpr();
  for (;;) {
    if (checkToken(TokenKind::Period)) {
      const auto memberIdentifier = currentToken.val;
      expectToken(TokenKind::Identifier);
      expr = std::make_unique<MemberRef>(std::move(expr),
                                         symbols.make(memberIdentifier));
    } else
      return expr;
  }
}

ExprPtr Parser::parsePrimaryExpr() {
  switch (currentToken.kind) {
  case TokenKind::String: {
    auto stringVal = currentToken.val;
    expectToken(TokenKind::String);
    return std::make_unique<StringLiteral>(std::move(stringVal));
  }
  case TokenKind::Number: {
    int val;
    try {
      val = std::stoi(currentToken.val);
    } catch (...) {
      // TODO: Improve reporting errors.
      throw ParserError("Conversion error");
    }
    expectToken(TokenKind::Number);
    return std::make_unique<NumberLiteral>(val);
  }
  case TokenKind::Identifier: {
    const auto identifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    // Check whether its a function call.
    if (checkToken(TokenKind::OpenParen)) {
      std::vector<ExprPtr> argList;
      while (!checkToken(TokenKind::CloseParen)) {
        while (!argList.empty())
          expectToken(TokenKind::Comma);
        argList.push_back(parseExpr());
      }
      return std::make_unique<Call>(symbols.make(identifier),
                                    std::move(argList));
    }
    return std::make_unique<VarRef>(symbols.make(identifier));
  }
  default:
#ifndef NDEBUG
    assert(!"Invalid primary expr");
#endif
    throw ParserError("Invalid primary expr");
  }
}

StatementPtr Parser::parseCompoundStatement() {
  std::vector<StatementPtr> body;
  while (!checkToken(TokenKind::End)) {
    // We could have a trailing semicolon after the last statement.
    // It isn't necessary but it's perfectly legal so let's check for it.
    if (checkToken(TokenKind::SemiColon) && checkToken(TokenKind::End))
      break;
    body.push_back(parseStatement());
  }
  return std::make_unique<Compound>(std::move(body));
}

StatementPtr Parser::parseIf() {
  auto cond = parseExpr();
  expectToken(TokenKind::Then);
  StatementPtr thenStatement = parseStatement(), elseStatement;
  if (checkToken(TokenKind::Else))
    elseStatement = parseStatement();
  return std::make_unique<If>(std::move(cond), std::move(thenStatement),
                              std::move(elseStatement));
}

StatementPtr Parser::parseCase() { return descartes::StatementPtr(); }

StatementPtr Parser::parseRepeat() { return descartes::StatementPtr(); }

StatementPtr Parser::parseWhile() {
  auto cond = parseExpr();
  expectToken(TokenKind::Do);
  auto body = parseStatement();
  return std::make_unique<While>(std::move(cond), std::move(body));
}

StatementPtr Parser::parseFor() {
  const auto controlIdentifier = currentToken.val;
  expectToken(TokenKind::Identifier);
  expectToken(TokenKind::Assign);
  auto beginExpr = parseExpr();
  bool to = checkToken(TokenKind::To);
  if (!to)
    expectToken(TokenKind::DownTo);
  auto endExpr = parseExpr();
  expectToken(TokenKind::Do);
  auto body = parseStatement();
  return std::make_unique<For>(symbols.make(controlIdentifier),
                               std::move(beginExpr), std::move(endExpr), to,
                               std::move(body));
}

StatementPtr Parser::parseWith() { return descartes::StatementPtr(); }

StatementPtr Parser::parseIdentifierStatement() {
  // This will either be an entire function call or the left hand side of an
  // assignment.
  auto expr = parseExpr();
  if (checkToken(TokenKind::Assign)) {
    auto rhs = parseExpr();
    return std::make_unique<Assignment>(std::move(expr), std::move(rhs));
  }
  return std::make_unique<CallStatement>(std::move(expr));
}

} // namespace descartes
