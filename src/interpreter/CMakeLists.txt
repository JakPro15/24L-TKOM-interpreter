add_library(
    Interpreter OBJECT
    include/includeExecution.hpp
    include/interpreterExceptions.hpp
    include/semanticAnalyzer.hpp
    includeExecution.cpp
    semanticAnalyzer.cpp
)
target_include_directories(Interpreter PUBLIC include)
target_compile_options(Interpreter PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(Interpreter Parser)
