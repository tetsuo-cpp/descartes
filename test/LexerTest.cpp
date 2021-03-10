#include <Lexer.h>

#include <catch2/catch.hpp>

namespace descartes::test {

// TODO: Check each token one by one and then have a custom printer to show the
// token mismatch.
void testLexer(const std::string &source, const std::vector<Token> &expected) {
  std::vector<Token> actual;
  Lexer lexer(source);
  while (auto token = lexer.lex())
    actual.push_back(std::move(token));
  for (size_t i = 0; i < std::min(expected.size(), actual.size()); ++i)
    REQUIRE(expected.at(i) == actual.at(i));
  REQUIRE(expected.size() == actual.size());
}

TEST_CASE("lex identifier", "[lexer]") {
  testLexer("foo", {Token(TokenKind::Identifier, "foo")});
}

TEST_CASE("lex identifier with number", "[lexer]") {
  testLexer("foo1", {Token(TokenKind::Identifier, "foo1")});
}

TEST_CASE("lex identifier followed by symbol", "[lexer]") {
  testLexer("foo;",
            {Token(TokenKind::Identifier, "foo"), Token(TokenKind::SemiColon)});
}

TEST_CASE("lex number", "[lexer]") {
  testLexer("123", {Token(TokenKind::Number, "123")});
}

TEST_CASE("lex string", "[lexer]") {
  testLexer("'foo'", {Token(TokenKind::String, "foo")});
}

TEST_CASE("lex symbols", "[lexer]") {
  testLexer(".;()",
            {Token(TokenKind::Period), Token(TokenKind::SemiColon),
             Token(TokenKind::OpenParen), Token(TokenKind::CloseParen)});
}

TEST_CASE("lex compound symbols", "[lexer]") {
  testLexer("<=>=<>",
            {Token(TokenKind::LessThanEqual),
             Token(TokenKind::GreaterThanEqual), Token(TokenKind::NotEqual)});
}

TEST_CASE("lex mixed symbols", "[lexer]") {
  testLexer("<;<=<(",
            {Token(TokenKind::LessThan), Token(TokenKind::SemiColon),
             Token(TokenKind::LessThanEqual), Token(TokenKind::LessThan),
             Token(TokenKind::OpenParen)});
}

TEST_CASE("lex keywords", "[lexer]") {
  testLexer("if foo begin end beginning",
            {Token(TokenKind::If), Token(TokenKind::Identifier, "foo"),
             Token(TokenKind::Begin), Token(TokenKind::End),
             Token(TokenKind::Identifier, "beginning")});
}

TEST_CASE("lex unknown symbol", "[lexer]") {
  Lexer lexer("?");
  REQUIRE_THROWS_MATCHES(lexer.lex(), descartes::LexerError,
                         Catch::Contains("Unknown symbol"));
}

TEST_CASE("lex mismatched quotes", "[lexer]") {
  Lexer lexer("'foo");
  REQUIRE_THROWS_MATCHES(lexer.lex(), descartes::LexerError,
                         Catch::Contains("Mismatched quotes"));
}

} // namespace descartes::test
