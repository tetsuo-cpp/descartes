set(
  DESCARTES_TEST_FILES
  LexerTest.cpp
  )

add_executable(descartes_test descartes_test.cpp ${DESCARTES_TEST_FILES})
find_package(Catch2 REQUIRED)
target_link_libraries(descartes_test descartes_lib Catch2::Catch2)
target_include_directories(descartes_test PRIVATE ../lib)
