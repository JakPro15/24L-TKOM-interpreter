add_library(
    Parser OBJECT
    include/documentTree.hpp
    include/parser.hpp
    include/documentTreeVisitor.hpp
    include/parserExceptions.hpp
    documentTree.cpp
    parser.cpp
)
target_include_directories(Parser PUBLIC include)
target_compile_options(Parser PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(Parser Reader)
target_link_libraries(Parser Lexer)
