#include <AstPrinter.h>
#include <Lexer.h>
#include <Parser.h>

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
  descartes::Parser parser(lexer);
  try {
    // Print the AST for debugging.
    auto program = parser.parse();
    descartes::AstPrinter printer;
    printer.printAst(*program);
  } catch (const descartes::LexerError &lexerError) {
    std::cerr << "LEXER: " << lexerError.what() << "\n";
  } catch (const descartes::ParserError &parserError) {
    std::cerr << "PARSER: " << parserError.what() << "\n";
  }
  return 0;
}
