#include <Lexer.h>
#include <Parser.h>
#include <Semantic.h>

#include <catch2/catch.hpp>

namespace descartes::test {

void testSemantic(const std::string &source, const std::string &msg) {
  Lexer lexer(source, false);
  Parser parser(lexer);
  auto program = parser.parse();
  Semantic semantic(parser.getSymbols());
  REQUIRE_THROWS_MATCHES(semantic.analyse(program), descartes::SemanticError,
                         Catch::Contains(msg));
}

} // namespace descartes::test
