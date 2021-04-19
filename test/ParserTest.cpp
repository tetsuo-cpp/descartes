#include <Lexer.h>
#include <Parser.h>

#include <catch2/catch.hpp>

namespace descartes::test {

void testParser(const std::string &source) {
  Lexer lexer(source, false);
  Parser parser(lexer);
  REQUIRE_NOTHROW(parser.parse());
}

TEST_CASE("parse hello world", "[parser]") {
  const char *program = "begin"
                        "  writeln('Hello, World!')"
                        "end.";
  testParser(program);
}

TEST_CASE("parse compound statement", "[parser]") {
  const char *program = "begin"
                        "  x := 1;"
                        "  y := 2"
                        "end.";
  testParser(program);
}

TEST_CASE("parse if-else statement", "[parser]") {
  const char *program = "begin"
                        "  if x = 1 then"
                        "    writeln('x is 1')"
                        "  else"
                        "    writeln('x is not 1')"
                        "end.";
  testParser(program);
}

TEST_CASE("parser operators", "[parser]") {
  const char *program = "begin"
                        "  x := x + y;"
                        "  x := x - y;"
                        "  x := x * y;"
                        "  x := x / y "
                        "end.";
  testParser(program);
}

TEST_CASE("parse block", "[parser]") {
  const char *program = "const"
                        "  x = 1;"
                        "  y = 2;"
                        "var"
                        "  z : integer;"
                        "begin "
                        "end.";
  testParser(program);
}

TEST_CASE("parse procedure", "[parser]") {
  const char *program = "procedure foo(x : integer);"
                        "const"
                        "  y = 1;"
                        "var"
                        "  z : integer;"
                        "begin"
                        "  z := x + y + 1;"
                        "  writeln(z)"
                        "end;"
                        "begin"
                        "  foo(1)"
                        "end.";
  testParser(program);
}

TEST_CASE("parse function", "[parser]") {
  const char *program = "function foo(x : integer) : integer;"
                        "begin"
                        "  foo := x "
                        "end;"
                        "begin"
                        "  writeln(foo(1))"
                        "end.";
  testParser(program);
}

TEST_CASE("parse const args", "[parser]") {
  const char *program = "function foo(const x : integer): integer;"
                        "begin"
                        "  foo := x "
                        "end;"
                        "begin "
                        "end.";
  testParser(program);
}

TEST_CASE("parse case statement", "[parser]") {
  const char *program = "begin"
                        "  case x of"
                        "    1: y := 'one';"
                        "    2: y := 'two';"
                        "    3: y := 'three'"
                        "  end "
                        "end.";
  testParser(program);
}

TEST_CASE("parse repeat-until statement", "[parser]") {
  const char *program = "begin"
                        "  repeat"
                        "    x := x + 1;"
                        "    y := y + 2"
                        "  until y <> 10 "
                        "end.";
  testParser(program);
}

} // namespace descartes::test
