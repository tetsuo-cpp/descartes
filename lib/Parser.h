#pragma once

#include <Interfaces.h>

#include <vector>

namespace descartes {

class Parser : public IParser {
public:
  explicit Parser(ILexer &lexer);
  virtual ~Parser() = default;
  AstPtr parse() override;

private:
  void readToken();
  bool isDone() const;
  bool checkToken(TokenKind kind);
  void expectToken(TokenKind kind);
  AstPtr parseBlock();
  std::vector<std::string> parseLabelDecl();
  std::vector<AstPtr> parseConstDef();
  AstPtr parseConstExpr();
  ILexer &lexer;
  Token currentToken;
};

} // namespace descartes
