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
  case TokenKind::LessThan:
    ss << "LessThan";
    break;
  case TokenKind::LessThanEqual:
    ss << "LessThanEqual";
    break;
  case TokenKind::GreaterThan:
    ss << "GreaterThan";
    break;
  case TokenKind::GreaterThanEqual:
    ss << "GreaterThanEqual";
    break;
  case TokenKind::Equal:
    ss << "Equal";
    break;
  case TokenKind::NotEqual:
    ss << "NotEqual";
    break;
  case TokenKind::If:
    ss << "If";
    break;
  case TokenKind::Eof:
    ss << "Eof";
    break;
  }
  ss << "\n";
  ss << "Value: " << (val.empty() ? "NONE" : val) << "\n";
  return ss.str();
}

Token::operator bool() const { return kind != TokenKind::Eof; }

bool Token::operator==(const Token &other) const {
  return kind == other.kind && val == other.val;
}

Lexer::Lexer(const std::string &source) : source(source), index(0) {
  readChar();
}

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

bool Lexer::isDone() const { return index > source.size(); }

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
  assert(std::isalpha(currentChar));
  std::string identifier{currentChar};
  while (readChar()) {
    if (!std::isalpha(currentChar) && !std::isdigit(currentChar))
      break;
    identifier.push_back(std::tolower(currentChar));
  }
  static const std::vector<std::pair<std::string, TokenKind>> keywordMap = {
      {"program", TokenKind::Program},
      {"begin", TokenKind::Begin},
      {"end", TokenKind::End},
      {"if", TokenKind::If},
  };
  // Check if the identifier is actually a keyword.
  const auto iter = std::find_if(
      keywordMap.begin(), keywordMap.end(),
      [&identifier](const std::pair<std::string, TokenKind> &keyword) {
        return identifier == keyword.first;
      });
  if (iter != keywordMap.end())
    return Token(iter->second);
  return Token(TokenKind::Identifier, std::move(identifier));
}

Token Lexer::lexNumber() {
  assert(std::isdigit(currentChar));
  std::string number{currentChar};
  while (readChar()) {
    if (!std::isdigit(currentChar))
      break;
    number.push_back(currentChar);
  }
  return Token(TokenKind::Number, std::move(number));
}

Token Lexer::lexString() {
  assert(currentChar == '\'');
  std::string stringLiteral;
  while (readChar()) {
    // TODO: Implement escaping.
    if (currentChar == '\'')
      break;
    stringLiteral.push_back(currentChar);
  }
  if (isDone())
    throw LexerError("Mismatched quotes");
  // Skip over the closing quote.
  readChar();
  return Token(TokenKind::String, std::move(stringLiteral));
}

Token Lexer::lexSymbol() {
  static std::vector<std::pair<std::string, TokenKind>> symbolMap = {
      {".", TokenKind::Period},      {";", TokenKind::SemiColon},
      {"(", TokenKind::OpenParen},   {")", TokenKind::CloseParen},
      {"<", TokenKind::LessThan},    {"<=", TokenKind::LessThanEqual},
      {">", TokenKind::GreaterThan}, {">=", TokenKind::GreaterThanEqual},
      {"=", TokenKind::Equal},       {"<>", TokenKind::NotEqual},
  };
  std::string currentSymbol;
  TokenKind kind = TokenKind::Eof;
  do {
    currentSymbol.push_back(currentChar);
    const auto iter = std::find_if(
        symbolMap.begin(), symbolMap.end(),
        [&currentSymbol](const std::pair<std::string, TokenKind> &symbol) {
          return currentSymbol == symbol.first;
        });
    if (iter == symbolMap.end())
      break;
    kind = iter->second;
  } while (readChar());
  // We didn't find a match.
  if (kind == TokenKind::Eof)
    throw LexerError("Unknown symbol");
  return Token(kind);
}

} // namespace descartes
