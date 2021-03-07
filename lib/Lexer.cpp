#include "Lexer.h"

#include <sstream>
#include <unordered_map>

namespace descartes {

std::string Token::toString() const {
  std::stringstream ss;
  ss << "Kind: ";
  switch (kind) {
  case TokenKind::Identifier:
    ss << "Identifier";
    break;
  case TokenKind::Number:
    ss << "Number";
    break;
  case TokenKind::String:
    ss << "String";
    break;
  case TokenKind::Program:
    ss << "Program";
    break;
  case TokenKind::Begin:
    ss << "Begin";
    break;
  case TokenKind::End:
    ss << "End";
    break;
  case TokenKind::Period:
    ss << "Period";
    break;
  case TokenKind::SemiColon:
    ss << "SemiColon";
    break;
  case TokenKind::OpenParen:
    ss << "OpenParen";
    break;
  case TokenKind::CloseParen:
    ss << "CloseParen";
    break;
  case TokenKind::Quote:
    ss << "Quote";
    break;
  case TokenKind::If:
    ss << "If";
    break;
  case TokenKind::Eof:
    ss << "Eof";
    break;
  case TokenKind::None:
    ss << "None";
    break;
  }
  ss << "\n";
  ss << "Value: " << (val.empty() ? "NONE" : val) << "\n";
  return ss.str();
}

Lexer::Lexer(const std::string &source)
    : source(source), index(0), currentChar(' ') {}

Token Lexer::lex() {
  trimWhitespace();
  if (isDone())
    return Token(TokenKind::Eof);
  if (std::isalpha(currentChar))
    return lexIdentifier();
  else if (std::isdigit(currentChar))
    return lexNumber();
  else if (currentChar == '\'')
    return lexString();
  else
    return lexSymbol();
}

bool Lexer::isDone() const { return index >= source.size(); }

Lexer::operator bool() const { return !isDone(); }

bool Lexer::readChar() {
  if (isDone())
    return false;
  currentChar = source[index++];
  return true;
}

void Lexer::trimWhitespace() {
  while (!isDone() && std::isspace(currentChar))
    readChar();
}

Token Lexer::lexIdentifier() {
  std::string identifier{currentChar};
  while (readChar()) {
    if (!std::isalpha(currentChar) && !std::isdigit(currentChar))
      break;
    identifier.push_back(currentChar);
  }
  return Token(TokenKind::Identifier, std::move(identifier));
}

Token Lexer::lexNumber() {
  std::string number{currentChar};
  while (readChar()) {
    if (std::isspace(currentChar))
      break;
    number.push_back(currentChar);
  }
  return Token(TokenKind::Number, std::move(number));
}

Token Lexer::lexString() {
  assert(currentChar == '\'');
  std::string stringLiteral;
  while (readChar()) {
    if (currentChar == '\'')
      break;
    stringLiteral.push_back(currentChar);
  }
  // Skip over the closing quote.
  readChar();
  return Token(TokenKind::String, std::move(stringLiteral));
}

Token Lexer::lexSymbol() {
  static std::unordered_map<char, TokenKind> symbolMap = {
      {'.', TokenKind::Period},    {';', TokenKind::SemiColon},
      {'(', TokenKind::OpenParen}, {')', TokenKind::CloseParen},
      {'\'', TokenKind::Quote},
  };
  const auto iter = symbolMap.find(currentChar);
  assert(iter != symbolMap.end());
  readChar();
  return Token(iter->second);
}

} // namespace descartes
