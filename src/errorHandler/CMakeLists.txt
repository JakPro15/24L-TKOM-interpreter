add_library(
    ErrorHandler OBJECT
    include/iErrorHandler.hpp
    include/errorHandler.hpp
    errorHandler.cpp
)
target_compile_options(ErrorHandler PUBLIC -fprofile-arcs -ftest-coverage)
