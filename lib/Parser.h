#pragma once

#include <Interfaces.h>
#include <SymbolTable.h>

#include <vector>

namespace descartes {

class Parser : public IParser {
public:
  explicit Parser(ILexer &lexer);
  virtual ~Parser() = default;
  Block parse() override;

private:
  void readToken();
  bool isDone() const;
  bool checkToken(TokenKind kind);
  void expectToken(TokenKind kind);
  Block parseBlock();
  std::vector<Symbol> parseLabelDecls();
  std::vector<ConstDef> parseConstDefs();
  ExprPtr parseConstExpr();
  TypeDefs parseTypeDefs();
  TypePtr parseType();
  TypePtr parseEnum();
  TypePtr parseRecord();
  std::vector<VarDecl> parseVarDecls();
  std::vector<std::unique_ptr<Function>> parseFunctions();
  std::unique_ptr<Function> parseProcedure();
  std::unique_ptr<Function> parseFunction();
  std::vector<FunctionArg> parseArgsList();
  StatementPtr parseStatement();
  ExprPtr parseExpr();
  ExprPtr parseEquality();
  ExprPtr parseRelational();
  ExprPtr parseAddition();
  ExprPtr parseMultiplication();
  ExprPtr parsePostfix();
  ExprPtr parsePrimaryExpr();
  StatementPtr parseCompoundStatement();
  StatementPtr parseIf();
  StatementPtr parseCase();
  StatementPtr parseRepeat();
  StatementPtr parseWhile();
  StatementPtr parseFor();
  StatementPtr parseWith();
  StatementPtr parseIdentifierStatement();
  ILexer &lexer;
  Token currentToken;
  SymbolTable symbols;
};

} // namespace descartes
