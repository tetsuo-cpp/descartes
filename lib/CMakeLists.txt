set(
  DESCARTES_LIB_FILES
  Ast.cpp
  AstPrinter.cpp
  Environment.cpp
  Interfaces.cpp
  Translate.cpp
  Lexer.cpp
  Parser.cpp
  Semantic.cpp
  SymbolTable.cpp
  )

add_library(descartes_lib ${DESCARTES_LIB_FILES})
target_include_directories(descartes_lib PRIVATE .)
