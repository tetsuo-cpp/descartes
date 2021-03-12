#include <Lexer.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace descartes {

std::string accumulateSource(std::ifstream &file) {
  std::string source, line;
  while (std::getline(file, line))
    source.append(line);
  return source;
}

} // namespace descartes

int main(int argc, char *argv[]) {
  if (argc != 2)
    return -1;
  const char *fileName = argv[1];
  std::ifstream file(fileName);
  const auto source = descartes::accumulateSource(file);
  descartes::Lexer lexer(source);
  std::vector<descartes::Token> tokens;
  try {
    while (auto token = lexer.lex()) {
      // TODO: Add a flag for this.
#ifndef NDEBUG
      std::cout << token.toString() << "\n";
#endif
      tokens.push_back(std::move(token));
    }
  } catch (const descartes::LexerError &lexerError) {
    std::cerr << lexerError.what() << "\n";
  }
  return 0;
}
