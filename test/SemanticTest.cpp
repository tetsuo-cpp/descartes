#include <Lexer.h>
#include <Parser.h>
#include <Semantic.h>

#include <catch2/catch.hpp>

namespace descartes::test {

void testSemanticSuccess(const std::string &source) {
  Lexer lexer(source, false);
  Parser parser(lexer);
  auto program = parser.parse();
  Semantic semantic(parser.getSymbols());
  REQUIRE_NOTHROW(semantic.analyse(program));
}

void testSemanticFailure(const std::string &source, const std::string &msg) {
  Lexer lexer(source, false);
  Parser parser(lexer);
  auto program = parser.parse();
  Semantic semantic(parser.getSymbols());
  REQUIRE_THROWS_MATCHES(semantic.analyse(program), descartes::SemanticError,
                         Catch::Contains(msg));
}

TEST_CASE("semantic hello world", "[semantic]") {
  const char *program = "begin"
                        "  writeln('Hello, world!')"
                        "end.";
  // FIXME: Once compiler intrinsics are implemented, this should work.
  testSemanticFailure(program, "Unknown function");
}

TEST_CASE("semantic integer assignment", "[semantic]") {
  const char *program = "var"
                        "  x: integer;"
                        "  y: integer;"
                        "begin"
                        "  x := 0;"
                        "  y := 1"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic string assignment", "[semantic]") {
  const char *program = "var"
                        "  x: string;"
                        "  y: string;"
                        "begin"
                        "  x := 'foo';"
                        "  y := 'bar'"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic struct member assignment", "[semantic]") {
  const char *program = "type"
                        "  TPerson = record"
                        "    name: string;"
                        "    age: integer"
                        "  end;"
                        "var"
                        "  person: TPerson;"
                        "begin "
                        "  person.name := 'Alex';"
                        "  person.age := 26"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic type error assignment 1", "[semantic]") {
  const char *program = "var"
                        "  x: integer;"
                        "begin"
                        "  x := 'foo'"
                        "end.";
  testSemanticFailure(program, "Assignment error");
}

TEST_CASE("semantic type error assignment 2", "[semantic]") {
  const char *program = "var"
                        "  x: string;"
                        "begin"
                        "  x := 1"
                        "end.";
  testSemanticFailure(program, "Assignment error");
}

TEST_CASE("semantic type error argument 1", "[semantic]") {
  const char *program = "var"
                        "  x: string;"
                        "procedure integerProcedure(x: integer);"
                        "begin "
                        "end;"
                        "begin"
                        "  x := 'foo';"
                        "  integerProcedure(x)"
                        "end.";
  testSemanticFailure(program, "Gave function wrong type");
}

TEST_CASE("semantic type error argument 2", "[semantic]") {
  const char *program = "type "
                        "  TPerson = record"
                        "    name: string;"
                        "    age: integer"
                        "  end;"
                        "var"
                        "  x: string;"
                        "procedure personProcedure(x: TPerson);"
                        "begin "
                        "end;"
                        "begin"
                        "  x := 'foo';"
                        "  personProcedure(x)"
                        "end.";
  testSemanticFailure(program, "Gave function wrong type");
}

TEST_CASE("semantic unknown function", "[semantic]") {
  const char *program = "begin"
                        "  unknownFunction()"
                        "end.";
  testSemanticFailure(program, "Unknown function");
}

TEST_CASE("semantic recursive call", "[semantic]") {
  const char *program = "function fib(x: integer): integer;"
                        "begin"
                        "  if x = 0 then"
                        "    fib := 0"
                        "  else if x = 1 then"
                        "    fib := 1"
                        "  else"
                        "    fib := fib(x - 1) + fib(x - 2)"
                        "end;"
                        "begin"
                        "  fib(10)"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic mutually recursive calls", "[semantic]") {
  const char *program = "procedure foo(x: integer);"
                        "begin"
                        "  bar(x)"
                        "end;"
                        "procedure bar(x: integer);"
                        "begin"
                        "  foo(x)"
                        "end;"
                        "begin"
                        "  foo(10)"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic unknown variable", "[semantic]") {
  const char *program = "begin"
                        "  x := 1"
                        "end.";
  testSemanticFailure(program, "unknown variable");
}

TEST_CASE("semantic compatible types 1", "[semantic]") {
  const char *program = "type"
                        "  TInteger1 = integer;"
                        "  TInteger2 = integer;"
                        "var"
                        "  int1: TInteger1;"
                        "  int2: TInteger2;"
                        "function add(x: TInteger1, y: TInteger2): integer;"
                        "begin"
                        "  add := x + y;"
                        "end;"
                        "begin"
                        "  int1 := 1;"
                        "  int2 := 2;"
                        "  int1 := add(int1, int2);"
                        "  int2 := add(int2, int1);"
                        "end.";
  testSemanticSuccess(program);
}

TEST_CASE("semantic compatible types 2", "[semantic]") {
  const char *program = "type"
                        "  TPerson = record"
                        "    name: string;"
                        "    age: integer;"
                        "  end;"
                        "  THuman = TPerson;"
                        "var"
                        "  person: TPerson;"
                        "  human: THuman;"
                        "procedure humanProc(x: THuman);"
                        "begin "
                        "end;"
                        "begin"
                        "  person.name := 'Alex';"
                        "  person.age := 26;"
                        "  human.name := 'Motoko';"
                        "  human.age := 56;"
                        "  humanProc(person);"
                        "  humanProc(human);"
                        "end.";
  testSemanticSuccess(program);
}

} // namespace descartes::test
