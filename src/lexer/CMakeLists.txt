add_library(
    Lexer OBJECT
    include/lexerExceptions.hpp
    include/tokenType.hpp
    include/token.hpp
    include/iLexer.hpp
    include/lexer.hpp
    lexer.cpp
    token.cpp
)
target_include_directories(Lexer PUBLIC include)
target_compile_options(Lexer PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(Lexer Reader)
