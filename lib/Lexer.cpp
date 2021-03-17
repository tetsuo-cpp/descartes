#include "Lexer.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace descartes {

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
      {"and", TokenKind::And},
      {"array", TokenKind::Array},
      {"begin", TokenKind::Begin},
      {"case", TokenKind::Case},
      {"const", TokenKind::Const},
      {"div", TokenKind::Div},
      {"do", TokenKind::Do},
      {"downto", TokenKind::DownTo},
      {"else", TokenKind::Else},
      {"end", TokenKind::End},
      {"file", TokenKind::File},
      {"for", TokenKind::For},
      {"function", TokenKind::Function},
      {"goto", TokenKind::GoTo},
      {"if", TokenKind::If},
      {"in", TokenKind::In},
      {"label", TokenKind::Label},
      {"mod", TokenKind::Mod},
      {"nil", TokenKind::Nil},
      {"not", TokenKind::Not},
      {"of", TokenKind::Of},
      {"or", TokenKind::Or},
      {"packed", TokenKind::Packed},
      {"procedure", TokenKind::Procedure},
      {"program", TokenKind::Program},
      {"record", TokenKind::Record},
      {"repeat", TokenKind::Repeat},
      {"set", TokenKind::Set},
      {"then", TokenKind::Then},
      {"to", TokenKind::To},
      {"type", TokenKind::Type},
      {"until", TokenKind::Until},
      {"var", TokenKind::Var},
      {"while", TokenKind::While},
      {"with", TokenKind::With},
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
      {"+", TokenKind::Add},
      {"-", TokenKind::Subtract},
      {"*", TokenKind::Multiply},
      {"/", TokenKind::Divide},
      {"=", TokenKind::Equal},
      {"<", TokenKind::LessThan},
      {">", TokenKind::GreaterThan},
      {"[", TokenKind::OpenBracket},
      {"]", TokenKind::CloseBracket},
      {".", TokenKind::Period},
      {",", TokenKind::Comma},
      {":", TokenKind::Colon},
      {";", TokenKind::SemiColon},
      {"^", TokenKind::Hat},
      {"(", TokenKind::OpenParen},
      {")", TokenKind::CloseParen},
      {"<>", TokenKind::NotEqual},
      {"<=", TokenKind::LessThanEqual},
      {">=", TokenKind::GreaterThanEqual},
      {":=", TokenKind::Assign},
      {"..", TokenKind::DoublePeriod},
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
