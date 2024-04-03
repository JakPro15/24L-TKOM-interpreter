add_library(
    Lexer OBJECT
    include/tokenType.hpp
    include/token.hpp
    include/lexer.hpp
    lexer.cpp
)
target_compile_options(Lexer PUBLIC -fprofile-arcs -ftest-coverage)
