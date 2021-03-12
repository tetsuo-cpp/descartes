#pragma once

#include <Interfaces.h>

namespace descartes {

class Parser : public IParser {
public:
  explicit Parser(ILexer &lexer);
  virtual ~Parser() = default;
  std::unique_ptr<IAst> parse() override;

private:
  ILexer &lexer;
};

} // namespace descartes
