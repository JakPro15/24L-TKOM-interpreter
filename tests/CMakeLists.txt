Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.3
)
FetchContent_GetProperties(Catch2)

if(NOT catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(
        "${catch2_SOURCE_DIR}"
        "${catch2_BINARY_DIR}"
    )
endif()

add_executable(
    UnitTests
    include/fakeLexer.hpp
    readerTest.cpp
    lexerTest.cpp
    commentDiscarderTest.cpp
)
target_compile_options(UnitTests PUBLIC -fprofile-arcs -ftest-coverage)
target_include_directories(UnitTests PUBLIC include)
target_link_options(UnitTests PUBLIC -lgcov --coverage)

target_link_libraries(UnitTests Reader)
target_link_libraries(UnitTests Lexer)
target_link_libraries(UnitTests Parser)
target_link_libraries(UnitTests Catch2::Catch2WithMain)

add_test(NAME UnitTests COMMAND UnitTests)
