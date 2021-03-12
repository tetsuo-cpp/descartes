#pragma once

#include <Interfaces.h>

#include <string>

namespace descartes {

class Lexer : public ILexer {
public:
  explicit Lexer(const std::string &source);
  virtual ~Lexer() = default;
  Token lex() override;

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

} // namespace descartes
