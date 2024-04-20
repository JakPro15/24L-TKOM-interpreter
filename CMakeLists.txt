cmake_minimum_required(VERSION 3.16.3)
project(TKOM_interpreter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SKIP_INSTALL_RULES True)

add_compile_options(-std=c++20 -Wall -Wextra -Wpedantic -Werror -Og -g)

enable_testing()

add_subdirectory(src/reader)
add_subdirectory(src/lexer)
add_subdirectory(tests)
