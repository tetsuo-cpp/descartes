#include "Parser.h"

#include <Ast.h>

namespace descartes {

Parser::Parser(ILexer &lexer) : lexer(lexer), currentToken(TokenKind::Eof) {
  readToken();
}

AstPtr Parser::parse() {
  if (isDone())
    return nullptr;
  return parseBlock();
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

AstPtr Parser::parseBlock() {
  std::vector<std::string> labelDecl;
  if (currentToken.kind == TokenKind::Label)
    labelDecl = parseLabelDecl();
  std::vector<AstPtr> constDef;
  if (currentToken.kind == TokenKind::Const)
    constDef = parseConstDef();
  AstPtr typeDef = nullptr, varDecl = nullptr, functions = nullptr,
         statements = nullptr;
  return std::make_unique<Block>(std::move(labelDecl), std::move(constDef),
                                 std::move(typeDef), std::move(varDecl),
                                 std::move(functions), std::move(statements));
}

std::vector<std::string> Parser::parseLabelDecl() {
  expectToken(TokenKind::Label);
  std::vector<std::string> labels;
  while (!checkToken(TokenKind::SemiColon)) {
    if (!labels.empty())
      expectToken(TokenKind::Comma);
    auto labelName = currentToken.val;
    expectToken(TokenKind::Identifier);
    labels.push_back(std::move(labelName));
  }
  return labels;
}

std::vector<AstPtr> Parser::parseConstDef() {
  expectToken(TokenKind::Const);
  std::vector<AstPtr> constDefs;
  // This marks the beginning of a subsequent section in the block. If we see
  // this, then get out.
  while (!isDone() && currentToken.kind != TokenKind::Type &&
         currentToken.kind != TokenKind::Var &&
         currentToken.kind != TokenKind::Begin) {
    auto identifier = currentToken.val;
    expectToken(TokenKind::Identifier);
    expectToken(TokenKind::Equal);
    auto constExpr = parseConstExpr();
    constDefs.push_back(std::make_unique<ConstDef>(std::move(identifier),
                                                   std::move(constExpr)));
    expectToken(TokenKind::SemiColon);
  }
  return constDefs;
}

AstPtr Parser::parseConstExpr() {
  switch (currentToken.kind) {
  case TokenKind::String: {
    auto stringLiteral = std::make_unique<StringLiteral>(currentToken.val);
    expectToken(TokenKind::String);
    return stringLiteral;
  }
  case TokenKind::Number: {
    int val;
    try {
      val = std::stoi(currentToken.val);
    } catch (...) {
      // TODO: Improve reporting errors.
      throw ParserError("Conversion error");
    }
    auto numberLiteral = std::make_unique<NumberLiteral>(val);
    expectToken(TokenKind::Number);
    return numberLiteral;
  }
  case TokenKind::Identifier: {
    auto variableRef = std::make_unique<VariableRef>(currentToken.val);
    expectToken(TokenKind::Identifier);
    return variableRef;
  }
  default:
    throw ParserError("Invalid const expr");
  }
}

} // namespace descartes
