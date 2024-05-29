add_library(
    Interpreter OBJECT
    include/includeExecution.hpp
    include/semanticExceptions.hpp
    include/runtimeExceptions.hpp
    include/semanticAnalysis.hpp
    include/interpreter.hpp
    include/builtinFunctions.hpp
    runtimeExceptions.cpp
    includeExecution.cpp
    semanticAnalysis.cpp
    interpreter.cpp
    builtinFunctions.cpp
)
target_include_directories(Interpreter PUBLIC include)
target_compile_options(Interpreter PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(Interpreter Parser)
