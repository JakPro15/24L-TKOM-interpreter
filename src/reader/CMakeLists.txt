add_library(
    Reader OBJECT
    include/readerExceptions.hpp
    include/position.hpp
    include/iReader.hpp
    include/streamReader.hpp
    readerExceptions.cpp
    streamReader.cpp
    position.cpp
)
target_compile_options(Reader PUBLIC -fprofile-arcs -ftest-coverage)
