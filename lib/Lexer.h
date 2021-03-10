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
  LessThan,
  LessThanEqual,
  GreaterThan,
  GreaterThanEqual,
  Equal,
  NotEqual,
  If,
  Eof,
};

struct Token {
  explicit Token(TokenKind kind) : kind(kind) {}
  template <typename T>
  Token(TokenKind kind, T &&val) : kind(kind), val(std::forward<T>(val)) {}
  explicit operator bool() const;
  bool operator==(const Token &other) const;
  std::string toString() const;
  TokenKind kind;
  std::string val;
};

class Lexer {
public:
  explicit Lexer(const std::string &source);
  virtual ~Lexer() = default;
  Token lex();

private:
  bool isDone() const;
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

class LexerError : public std::runtime_error {
public:
  template <typename T>
  explicit LexerError(T &&msg) : std::runtime_error(std::forward<T>(msg)) {}
  virtual ~LexerError() = default;
  operator std::string() const { return std::runtime_error::what(); }
};

} // namespace descartes
