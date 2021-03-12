#pragma once

#include <string>

namespace descartes {

enum class TokenKind {
  Identifier,
  Number,
  String,
  // Symbols.
  Add,
  Subtract,
  Multiply,
  Divide,
  Equal,
  LessThan,
  GreaterThan,
  OpenBracket,
  CloseBracket,
  Period,
  Comma,
  Colon,
  SemiColon,
  Hat,
  OpenParen,
  CloseParen,
  NotEqual,
  LessThanEqual,
  GreaterThanEqual,
  Assign,
  DoublePeriod,
  // Keywords.
  And,
  Array,
  Begin,
  Case,
  Const,
  Div,
  Do,
  DownTo,
  Else,
  End,
  File,
  For,
  Function,
  GoTo,
  If,
  In,
  Label,
  Mod,
  Nil,
  Not,
  Of,
  Or,
  Packed,
  Procedure,
  Program,
  Record,
  Repeat,
  Set,
  Then,
  To,
  Type,
  Until,
  Var,
  While,
  With,
  // End of token stream.
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

class ILexer {
public:
  virtual ~ILexer() = default;
  virtual Token lex() = 0;
};

class LexerError : public std::runtime_error {
public:
  template <typename T>
  explicit LexerError(T &&msg) : std::runtime_error(std::forward<T>(msg)) {}
  virtual ~LexerError() = default;
  operator std::string() const;
};

class IAst {};

class IParser {
public:
  virtual ~IParser() = default;
  virtual std::unique_ptr<IAst> parse() = 0;
};

class ParserError : public std::runtime_error {
public:
  template <typename T>
  explicit ParserError(T &&msg) : std::runtime_error(std::forward<T>(msg)) {}
  virtual ~ParserError() = default;
  operator std::string() const;
};

} // namespace descartes
