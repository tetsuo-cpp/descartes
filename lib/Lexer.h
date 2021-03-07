#pragma once

#include <string>
#include <vector>

namespace descartes {

enum class TokenKind {
  Identifier,
  Number,
  String,
  Program,
  Begin,
  End,
  Period,
  SemiColon,
  OpenParen,
  CloseParen,
  Quote,
  If,
  Eof,
  None,
};

struct Token {
  Token() : kind(TokenKind::None) {}
  explicit Token(TokenKind kind) : kind(kind) {}
  template <typename T>
  Token(TokenKind kind, T &&val) : kind(kind), val(std::forward<T>(val)) {}
  std::string toString() const;
  TokenKind kind;
  std::string val;
};

class Lexer {
public:
  explicit Lexer(const std::string &source);
  virtual ~Lexer() = default;
  Token lex();
  bool isDone() const;
  explicit operator bool() const;

private:
  bool readChar();
  void trimWhitespace();
  Token lexIdentifier();
  Token lexNumber();
  Token lexString();
  Token lexSymbol();
  const std::string &source;
  size_t index;
  char currentChar;
};

} // namespace descartes
