add_library(
    Parser OBJECT
    include/documentTree.hpp
    include/type.hpp
    include/parser.hpp
    include/documentTreeVisitor.hpp
    include/parserExceptions.hpp
    include/printingVisitor.hpp
    type.cpp
    object.cpp
    documentTree.cpp
    parser.cpp
    printingVisitor.cpp
)
target_include_directories(Parser PUBLIC include)
target_compile_options(Parser PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(Parser Reader)
target_link_libraries(Parser Lexer)
