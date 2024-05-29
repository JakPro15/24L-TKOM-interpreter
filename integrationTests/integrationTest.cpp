#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>

void checkFileContents(const std::string &fileName, const std::string &expected)
{
    std::ifstream outputStream(fileName);
    REQUIRE(outputStream.is_open());
    std::string output;
    output.resize(expected.size() + 200);
    outputStream.read(&output[0], expected.size() + 200);
    output.resize(outputStream.gcount());
    REQUIRE(output == expected);
}

enum class SuccessEnum
{
    Succeeded,
    Failed
};

using enum SuccessEnum;

void checkOutputFromArgs(
    const std::string &programArgs, const std::string &expectedOutput, const std::string &expectedError,
    SuccessEnum success, const std::string &inputFile = ""
)
{
    std::string inputPipe = inputFile.empty() ? "" : ("cat " + inputFile + " | ");
    int errorCode = std::system((inputPipe + "./src/app/inter " + programArgs + " 1> output.txt 2> error.txt").c_str());
    checkFileContents("error.txt", expectedError);
    checkFileContents("output.txt", expectedOutput);
    if(success == Succeeded)
        REQUIRE(errorCode == 0);
    else
        REQUIRE(errorCode != 0);
}

TEST_CASE("command line interface errors", "[IntegrationTests]")
{
    checkOutputFromArgs(
        "", "",
        "The interpreter's command line interface encountered an error:\n"
        "No source code files given to interpreter\n",
        Failed
    );
    checkOutputFromArgs(
        "file1.txt file1.txt", "",
        "The interpreter's command line interface encountered an error:\n"
        "File name given to interpreter more than once: file1.txt\n",
        Failed
    );
    checkOutputFromArgs(
        "file1.txt", "",
        "The interpreter's command line interface encountered an error:\n"
        "Failed to open file file1.txt\n",
        Failed
    );
}

TEST_CASE("factorial program with one include statement", "[IntegrationTests]")
{
    checkOutputFromArgs("test1/factorialUse.txt --args 3", "6\n", "", Succeeded);
    checkOutputFromArgs("test1/factorial.txt --args 4", "24\n", "", Succeeded);
    checkOutputFromArgs("test1/factorialUse.txt --args 10", "3628800\n", "", Succeeded);
    checkOutputFromArgs("test1/factorial.txt", "no argument given\n", "", Succeeded);
    checkOutputFromArgs(
        "test1/factorialUse.txt --args -1", "",
        "The program was terminated following a runtime error:\n"
        "Recursion limit exceeded\n"
        "while executing file test1/factorial.txt\n"
        "at line 6, column 20.\n",
        Failed
    );
    checkOutputFromArgs(
        "test1/factorial.txt test1/factorialUse.txt --args abc", "",
        "The program was terminated following a runtime error:\n"
        "Conversion of string abc to integer failed\n"
        "while executing file test1/factorialUse.txt\n"
        "at line 8, column 23.\n",
        Failed
    );
}

TEST_CASE("programs with non-runtime errors", "[IntegrationTests]")
{
    checkOutputFromArgs(
        "test2/syntaxError.txt", "",
        "Error: Expected '(', got '}'\n"
        "in file test2/syntaxError.txt\n"
        "at line 2, column 1.\n",
        Failed
    );
    checkOutputFromArgs(
        "test2/includedError.txt", "",
        "Error: Expected '(', got '}'\n"
        "in file test2/syntaxError.txt\n"
        "at line 2, column 1.\n",
        Failed
    );
    checkOutputFromArgs(
        "test2/collision1.txt", "",
        "Error: Duplicate function with signature input\n"
        "in file test2/collision1.txt\n"
        "at line 1, column 1.\n",
        Failed
    );
    checkOutputFromArgs(
        "test2/collision2.txt", "",
        "Error: Duplicate function with signature main\n"
        "in file test1/factorialUse.txt\n"
        "at line 3, column 1.\n",
        Failed
    );
}

TEST_CASE("line adder program - reading from stdin", "[IntegrationTests]")
{
    checkOutputFromArgs(
        "test3/lineNumberAdder.txt", "1 first line\n2 second line\n3 third line\n", "", Succeeded, "test3/input.txt"
    );
    checkOutputFromArgs(
        "test3/lineNumberAdder.txt --args a", "a1 first line\na2 second line\na3 third line\n", "", Succeeded,
        "test3/input.txt"
    );
}
