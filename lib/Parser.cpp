#include "Parser.h"

namespace descartes {

Parser::Parser(ILexer &lexer) : lexer(lexer) {}

std::unique_ptr<IAst> Parser::parse() {
  static_cast<void>(lexer);
  return nullptr;
}

} // namespace descartes
