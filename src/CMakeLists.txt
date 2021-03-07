add_executable(descartes descartes.cpp)
target_link_libraries(descartes descartes_lib)
target_include_directories(descartes PRIVATE ../lib)
