set(
  DESCARTES_LIB_FILES
  Ast.cpp
  AstPrinter.cpp
  Interfaces.cpp
  Lexer.cpp
  Parser.cpp
  SymbolTable.cpp
  )

add_library(descartes_lib ${DESCARTES_LIB_FILES})
target_include_directories(descartes_lib PRIVATE .)
