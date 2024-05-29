#include "argumentParsing.hpp"

#include "appExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("with --dump-dt", "[parseArguments]")
{
    const char *argv[] = {"execname", "file1.txt", "file2.txt", "--dump-dt", "file3.txt", "--args", "arg1", "arg2"};
    Arguments arguments = parseArguments(sizeof(argv) / sizeof(const char *), argv);
    REQUIRE(arguments.files == std::vector<std::wstring>{L"file1.txt", L"file2.txt", L"file3.txt"});
    REQUIRE(arguments.dumpDocumentTree == true);
    REQUIRE(arguments.programArguments == std::vector<std::wstring>{L"arg1", L"arg2"});
}

TEST_CASE("without --dump-dt", "[parseArguments]")
{
    const char *argv[] = {"execname", "arg1", "arg2", "--args", "file1.txt", "file2.txt"};
    Arguments arguments = parseArguments(sizeof(argv) / sizeof(const char *), argv);
    REQUIRE(arguments.files == std::vector<std::wstring>{L"arg1", L"arg2"});
    REQUIRE(arguments.dumpDocumentTree == false);
    REQUIRE(arguments.programArguments == std::vector<std::wstring>{L"file1.txt", L"file2.txt"});
}

TEST_CASE("no files given", "[parseArguments]")
{
    const char *argv[] = {"execname", "--dump-dt", "--args", "file1.txt", "file2.txt"};
    REQUIRE_THROWS_AS(parseArguments(sizeof(argv) / sizeof(const char *), argv), NoFilesError);
}

TEST_CASE("duplicate file name given", "[parseArguments]")
{
    const char *argv[] = {"execname", "file1.txt", "--dump-dt", "file1.txt", "--args", "file1.txt", "file2.txt"};
    REQUIRE_THROWS_AS(parseArguments(sizeof(argv) / sizeof(const char *), argv), DuplicateFileError);
}
