set(
  DESCARTES_TEST_FILES
  LexerTest.cpp
  )

add_executable(descartes_test descartes_test.cpp ${DESCARTES_TEST_FILES})
target_link_libraries(descartes_test descartes_lib ${CONAN_LIBS})
target_include_directories(descartes_test PRIVATE ../lib)
