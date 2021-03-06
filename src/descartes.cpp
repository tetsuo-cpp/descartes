#include <AstPrinter.h>
#include <Lexer.h>
#include <Parser.h>
#include <Semantic.h>

#include <argparse/argparse.hpp>

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
  argparse::ArgumentParser argParser("descartes");
  argParser.add_argument("file").help("the source file to compile");
  argParser.add_argument("--print_tokens")
      .help("print the tokens generated by the lexer")
      .default_value(false)
      .implicit_value(true);
  argParser.add_argument("--print_ast")
      .help("print the ast generated by the parser")
      .default_value(false)
      .implicit_value(true);
  try {
    argParser.parse_args(argc, argv);
  } catch (const std::runtime_error &argParseError) {
    std::cerr << argParseError.what() << "\n";
    return -1;
  }
  const auto fileName = argParser.get<std::string>("file");
  const bool printTokens = argParser.get<bool>("--print_tokens");
  const bool printAst = argParser.get<bool>("--print_ast");
  std::ifstream file(fileName);
  if (!file) {
    std::cerr << "Could not open file " << fileName << "\n";
    return -1;
  }
  // TODO: Extract into driver component.
  const auto source = descartes::accumulateSource(file);
  descartes::Lexer lexer(source, printTokens);
  descartes::Parser parser(lexer);
  try {
    // Print the AST for debugging.
    auto program = parser.parse();
    if (printAst) {
      descartes::AstPrinter printer;
      printer.printBlock(program);
    }
    descartes::Semantic semantic(parser.getSymbols());
    const auto &frags = semantic.analyse(program);
    static_cast<void>(frags);
  } catch (const descartes::LexerError &lexerError) {
    std::cerr << "LEXER: " << lexerError.what() << "\n";
  } catch (const descartes::ParserError &parserError) {
    std::cerr << "PARSER: " << parserError.what() << "\n";
  } catch (const descartes::SemanticError &semanticError) {
    std::cerr << "SEMANTIC: " << semanticError.what() << "\n";
  }
  return 0;
}
