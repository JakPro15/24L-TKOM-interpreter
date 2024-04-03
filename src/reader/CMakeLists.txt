add_library(
    Reader OBJECT
    include/position.hpp
    include/ireader.hpp
    include/streamReader.hpp
    streamReader.cpp
    position.cpp
)
target_compile_options(Reader PUBLIC -fprofile-arcs -ftest-coverage)
