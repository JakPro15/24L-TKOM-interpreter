cmake_minimum_required(VERSION 3.16.3)
project(TKOM_interpreter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SKIP_INSTALL_RULES True)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Og -g)

include_directories(src/lexer)

enable_testing()

add_subdirectory(src/lexer)
