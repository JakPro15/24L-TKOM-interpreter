add_library(
    SemanticAnalyzer OBJECT
    include/semanticAnalyzer.hpp
    semanticAnalyzer.cpp
)
target_include_directories(SemanticAnalyzer PUBLIC include)
target_compile_options(SemanticAnalyzer PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(SemanticAnalyzer Parser)
