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

TEST_CASE("document tree dumping", "[IntegrationTests]")
{
    std::string lineNumberAdderDt = "Program containing:\n"
                                    "Functions:\n"
                                    "`-main: FunctionDeclaration <line: 1, col: 1> source=test3/lineNumberAdder.txt\n"
                                    " `-Body:\n"
                                    "  |-VariableDeclStatement <line: 2, col: 5>\n"
                                    "  ||-VariableDeclaration <line: 2, col: 5> type=str name=start mutable=true\n"
                                    "  |`-Literal <line: 2, col: 18> type=str value=\n"
                                    "  |-IfStatement <line: 3, col: 5>\n"
                                    "  |`-SingleIfCase <line: 3, col: 5>\n"
                                    "  | |-GreaterExpression <line: 3, col: 8>\n"
                                    "  | ||-FunctionCall <line: 3, col: 8> functionName=no_arguments\n"
                                    "  | |`-Literal <line: 3, col: 25> type=int value=0\n"
                                    "  | `-AssignmentStatement <line: 4, col: 9>\n"
                                    "  |  |-Assignable <line: 4, col: 9> right=start\n"
                                    "  |  `-FunctionCall <line: 4, col: 17> functionName=argument\n"
                                    "  |   `-Literal <line: 4, col: 26> type=int value=0\n"
                                    "  |-VariableDeclStatement <line: 6, col: 5>\n"
                                    "  ||-VariableDeclaration <line: 6, col: 5> type=int name=i mutable=true\n"
                                    "  |`-Literal <line: 6, col: 14> type=int value=1\n"
                                    "  |-VariableDeclStatement <line: 7, col: 5>\n"
                                    "  ||-VariableDeclaration <line: 7, col: 5> type=str name=line mutable=true\n"
                                    "  |`-FunctionCall <line: 7, col: 17> functionName=input\n"
                                    "  `-WhileStatement <line: 8, col: 5>\n"
                                    "   |-GreaterExpression <line: 8, col: 11>\n"
                                    "   ||-FunctionCall <line: 8, col: 11> functionName=len\n"
                                    "   ||`-Variable <line: 8, col: 15> name=line\n"
                                    "   |`-Literal <line: 8, col: 23> type=int value=0\n"
                                    "   |-FunctionCallInstruction <line: 9, col: 9>\n"
                                    "   |`-FunctionCall <line: 9, col: 9> functionName=print\n"
                                    "   | `-ConcatExpression <line: 9, col: 15>\n"
                                    "   |  |-ConcatExpression <line: 9, col: 15>\n"
                                    "   |  ||-ConcatExpression <line: 9, col: 15>\n"
                                    "   |  |||-ConcatExpression <line: 9, col: 15>\n"
                                    "   |  ||||-Variable <line: 9, col: 15> name=start\n"
                                    "   |  |||`-Variable <line: 9, col: 23> name=i\n"
                                    "   |  ||`-Literal <line: 9, col: 27> type=str value= \n"
                                    "   |  |`-Variable <line: 9, col: 33> name=line\n"
                                    "   |  `-Literal <line: 9, col: 40> type=str value=\n\n"
                                    "   |-AssignmentStatement <line: 10, col: 9>\n"
                                    "   ||-Assignable <line: 10, col: 9> right=line\n"
                                    "   |`-FunctionCall <line: 10, col: 16> functionName=input\n"
                                    "   `-AssignmentStatement <line: 11, col: 9>\n"
                                    "    |-Assignable <line: 11, col: 9> right=i\n"
                                    "    `-PlusExpression <line: 11, col: 13>\n"
                                    "     |-Variable <line: 11, col: 13> name=i\n"
                                    "     `-Literal <line: 11, col: 17> type=int value=1\n";
    checkOutputFromArgs("test3/lineNumberAdder.txt --dump-dt", lineNumberAdderDt, "", Succeeded);
}
