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
    Tests
    include/fakeLexer.hpp
    include/helpers.hpp
    helpers.cpp
    readerTest.cpp
    lexerTest.cpp
    commentDiscarderTest.cpp
    parserTest.cpp
    lexerAndParserTest.cpp
    semanticAnalysisTest.cpp
    lexerParserSemanticTest.cpp
    builtinFunctionsTest.cpp
    includeExecutionTest.cpp
    interpreterTest.cpp
    lexerToInterpreterTest.cpp
)
target_compile_options(Tests PUBLIC -fprofile-arcs -ftest-coverage)
target_include_directories(Tests PUBLIC include)
target_link_options(Tests PUBLIC -lgcov --coverage)

target_link_libraries(Tests Reader)
target_link_libraries(Tests Lexer)
target_link_libraries(Tests Parser)
target_link_libraries(Tests Interpreter)
target_link_libraries(Tests Catch2::Catch2WithMain)

add_test(NAME Tests COMMAND Tests)
