#include "commentDiscarder.hpp"
#include "helpers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "parserExceptions.hpp"
#include "printingVisitor.hpp"
#include "streamReader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>

namespace {
Program getTree(const std::wstring &source)
{
    std::wstringstream sourceStream(source);
    StreamReader reader(sourceStream, L"<test>");
    Lexer lexer(reader);
    CommentDiscarder commentDiscarder(lexer);
    Parser parser(commentDiscarder);
    return parser.parseProgram();
}

void checkLexingAndParsing(
    const std::wstring &source, const std::set<std::wstring> &expectedStructs,
    const std::set<std::wstring> &expectedVariants, const std::set<std::wstring> &expectedFunctions
)
{
    Program tree = getTree(source);

    checkNodeContainer(tree.structs, expectedStructs);
    checkNodeContainer(tree.variants, expectedVariants);
    checkNodeContainer(tree.functions, expectedFunctions);
}
}

TEST_CASE("overloaded functions example", "[Lexer+Parser]")
{
    std::wstring source = L"variant V {int a; str b; bool c;}\n"
                          L"func f(int v) -> int { return 1; }\n"
                          L"func f(str v) -> int { return 2; }\n"
                          L"func f(bool v) -> int { return 3; }\n"
                          L"\n"
                          L"func main() {\n"
                          L"    V vart1 = 2;\n"
                          L"    V vart2 = \"string\";\n"
                          L"    print(f(vart1)); # 1\n"
                          L"    print(f(vart2)); # 2\n"
                          L"}\n";
    std::set<std::wstring> expectedVariants = {L"V: VariantDeclaration <line: 1, col: 1> source=<test>\n"
                                               L"|-Field <line: 1, col: 12> type=int name=a\n"
                                               L"|-Field <line: 1, col: 19> type=str name=b\n"
                                               L"`-Field <line: 1, col: 26> type=bool name=c\n"};
    std::set<std::wstring> expectedFunctions = {
        L"f(int): FunctionDeclaration <line: 2, col: 1> source=<test> returnType=int\n"
        L"|-Parameters:\n"
        L"|`-VariableDeclaration <line: 2, col: 8> type=int name=v mutable=false\n"
        L"`-Body:\n"
        L" `-ReturnStatement <line: 2, col: 24>\n"
        L"  `-Literal <line: 2, col: 31> type=int value=1\n",
        L"f(str): FunctionDeclaration <line: 3, col: 1> source=<test> returnType=int\n"
        L"|-Parameters:\n"
        L"|`-VariableDeclaration <line: 3, col: 8> type=str name=v mutable=false\n"
        L"`-Body:\n"
        L" `-ReturnStatement <line: 3, col: 24>\n"
        L"  `-Literal <line: 3, col: 31> type=int value=2\n",
        L"f(bool): FunctionDeclaration <line: 4, col: 1> source=<test> returnType=int\n"
        L"|-Parameters:\n"
        L"|`-VariableDeclaration <line: 4, col: 8> type=bool name=v mutable=false\n"
        L"`-Body:\n"
        L" `-ReturnStatement <line: 4, col: 25>\n"
        L"  `-Literal <line: 4, col: 32> type=int value=3\n",
        L"main: FunctionDeclaration <line: 6, col: 1> source=<test>\n"
        L"`-Body:\n"
        L" |-VariableDeclStatement <line: 7, col: 5>\n"
        L" ||-VariableDeclaration <line: 7, col: 5> type=V name=vart1 mutable=false\n"
        L" |`-Literal <line: 7, col: 15> type=int value=2\n"
        L" |-VariableDeclStatement <line: 8, col: 5>\n"
        L" ||-VariableDeclaration <line: 8, col: 5> type=V name=vart2 mutable=false\n"
        L" |`-Literal <line: 8, col: 15> type=str value=string\n"
        L" |-FunctionCallInstruction <line: 9, col: 5>\n"
        L" |`-FunctionCall <line: 9, col: 5> functionName=print\n"
        L" | `-FunctionCall <line: 9, col: 11> functionName=f\n"
        L" |  `-Variable <line: 9, col: 13> name=vart1\n"
        L" `-FunctionCallInstruction <line: 10, col: 5>\n"
        L"  `-FunctionCall <line: 10, col: 5> functionName=print\n"
        L"   `-FunctionCall <line: 10, col: 11> functionName=f\n"
        L"    `-Variable <line: 10, col: 13> name=vart2\n"
    };
    checkLexingAndParsing(source, {}, expectedVariants, expectedFunctions);
}

TEST_CASE("structs and variants example", "[Lexer+Parser]")
{
    std::wstring source = L"struct S1 {\n"
                          L"    int a;\n"
                          L"    int b;\n"
                          L"}\n"
                          L"variant V {\n"
                          L"    S1 s;\n"
                          L"    int i;\n"
                          L"}\n"
                          L"struct S2 {\n"
                          L"    V v;\n"
                          L"    int i;\n"
                          L"}\n"
                          L"\n"
                          L"func main() {\n"
                          L"    S2 s = {S1({2, 3}), 2};\n"
                          L"    # konieczne jest podanie typu S1, ponieważ jest wewnątrz rekordu wariantowego\n"
                          L"}\n";
    std::set<std::wstring> expectedStructs = {
        L"S1: StructDeclaration <line: 1, col: 1> source=<test>\n"
        L"|-Field <line: 2, col: 5> type=int name=a\n"
        L"`-Field <line: 3, col: 5> type=int name=b\n",
        L"S2: StructDeclaration <line: 9, col: 1> source=<test>\n"
        L"|-Field <line: 10, col: 5> type=V name=v\n"
        L"`-Field <line: 11, col: 5> type=int name=i\n"
    };
    std::set<std::wstring> expectedVariants = {L"V: VariantDeclaration <line: 5, col: 1> source=<test>\n"
                                               L"|-Field <line: 6, col: 5> type=S1 name=s\n"
                                               L"`-Field <line: 7, col: 5> type=int name=i\n"};
    std::set<std::wstring> expectedFunctions = {
        L"main: FunctionDeclaration <line: 14, col: 1> source=<test>\n"
        L"`-Body:\n"
        L" `-VariableDeclStatement <line: 15, col: 5>\n"
        L"  |-VariableDeclaration <line: 15, col: 5> type=S2 name=s mutable=false\n"
        L"  `-StructExpression <line: 15, col: 12>\n"
        L"   |-FunctionCall <line: 15, col: 13> functionName=S1\n"
        L"   |`-StructExpression <line: 15, col: 16>\n"
        L"   | |-Literal <line: 15, col: 17> type=int value=2\n"
        L"   | `-Literal <line: 15, col: 20> type=int value=3\n"
        L"   `-Literal <line: 15, col: 25> type=int value=2\n"
    };
    checkLexingAndParsing(source, expectedStructs, expectedVariants, expectedFunctions);
}

TEST_CASE("loops example", "[Lexer+Parser]")
{
    std::wstring source = L"func main() {\n"
                          L"    int$ a = 0;\n"
                          L"    int$ b = 0;\n"
                          L"    while(a < 10) {\n"
                          L"        do {\n"
                          L"            b = b + 1;\n"
                          L"            if(a > 4) {\n"
                          L"                continue;\n"
                          L"            }\n"
                          L"            a = a + 1;\n"
                          L"        } while(b < 10)\n"
                          L"        if(b == 13) {\n"
                          L"            break;\n"
                          L"        }\n"
                          L"        a = a + 1;\n"
                          L"    }\n"
                          L"# a === 8, b === 13\n"
                          L"}\n";
    std::set<std::wstring> expectedFunctions = {
        L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
        L"`-Body:\n"
        L" |-VariableDeclStatement <line: 2, col: 5>\n"
        L" ||-VariableDeclaration <line: 2, col: 5> type=int name=a mutable=true\n"
        L" |`-Literal <line: 2, col: 14> type=int value=0\n"
        L" |-VariableDeclStatement <line: 3, col: 5>\n"
        L" ||-VariableDeclaration <line: 3, col: 5> type=int name=b mutable=true\n"
        L" |`-Literal <line: 3, col: 14> type=int value=0\n"
        L" `-WhileStatement <line: 4, col: 5>\n"
        L"  |-LesserExpression <line: 4, col: 11>\n"
        L"  ||-Variable <line: 4, col: 11> name=a\n"
        L"  |`-Literal <line: 4, col: 15> type=int value=10\n"
        L"  |-DoWhileStatement <line: 5, col: 9>\n"
        L"  ||-LesserExpression <line: 11, col: 17>\n"
        L"  |||-Variable <line: 11, col: 17> name=b\n"
        L"  ||`-Literal <line: 11, col: 21> type=int value=10\n"
        L"  ||-AssignmentStatement <line: 6, col: 13>\n"
        L"  |||-Assignable <line: 6, col: 13> right=b\n"
        L"  ||`-PlusExpression <line: 6, col: 17>\n"
        L"  || |-Variable <line: 6, col: 17> name=b\n"
        L"  || `-Literal <line: 6, col: 21> type=int value=1\n"
        L"  ||-IfStatement <line: 7, col: 13>\n"
        L"  ||`-SingleIfCase <line: 7, col: 13>\n"
        L"  || |-GreaterExpression <line: 7, col: 16>\n"
        L"  || ||-Variable <line: 7, col: 16> name=a\n"
        L"  || |`-Literal <line: 7, col: 20> type=int value=4\n"
        L"  || `-ContinueStatement <line: 8, col: 17>\n"
        L"  |`-AssignmentStatement <line: 10, col: 13>\n"
        L"  | |-Assignable <line: 10, col: 13> right=a\n"
        L"  | `-PlusExpression <line: 10, col: 17>\n"
        L"  |  |-Variable <line: 10, col: 17> name=a\n"
        L"  |  `-Literal <line: 10, col: 21> type=int value=1\n"
        L"  |-IfStatement <line: 12, col: 9>\n"
        L"  |`-SingleIfCase <line: 12, col: 9>\n"
        L"  | |-EqualExpression <line: 12, col: 12>\n"
        L"  | ||-Variable <line: 12, col: 12> name=b\n"
        L"  | |`-Literal <line: 12, col: 17> type=int value=13\n"
        L"  | `-BreakStatement <line: 13, col: 13>\n"
        L"  `-AssignmentStatement <line: 15, col: 9>\n"
        L"   |-Assignable <line: 15, col: 9> right=a\n"
        L"   `-PlusExpression <line: 15, col: 13>\n"
        L"    |-Variable <line: 15, col: 13> name=a\n"
        L"    `-Literal <line: 15, col: 17> type=int value=1\n"
    };
    checkLexingAndParsing(source, {}, {}, expectedFunctions);
}

void doPriorityTest(const std::wstring &expressionSource, const std::wstring &expectedMain)
{
    std::wstring source = L"func main() { print(" + expressionSource + L"); }";
    Program tree = getTree(source);
    REQUIRE(tree.functions.size() == 1);
    std::unique_ptr<BaseFunctionDeclaration> &main = tree.functions.at({L"main", {}});

    std::wstringstream result;
    PrintingVisitor printer(result);
    main->accept(printer);
    REQUIRE(result.str() == expectedMain);
}

TEST_CASE("priority tests", "[Lexer+Parser]")
{
    doPriorityTest(
        L"2 / 2 * 3", L"FunctionDeclaration <line: 1, col: 1> source=<test>\n"
                      L"`-Body:\n"
                      L" `-FunctionCallInstruction <line: 1, col: 15>\n"
                      L"  `-FunctionCall <line: 1, col: 15> functionName=print\n"
                      L"   `-MultiplyExpression <line: 1, col: 21>\n"
                      L"    |-DivideExpression <line: 1, col: 21>\n"
                      L"    ||-Literal <line: 1, col: 21> type=int value=2\n"
                      L"    |`-Literal <line: 1, col: 25> type=int value=2\n"
                      L"    `-Literal <line: 1, col: 29> type=int value=3\n"
    );
    doPriorityTest(
        L"\"4\" ** 2 ** \"1\"", L"FunctionDeclaration <line: 1, col: 1> source=<test>\n"
                                L"`-Body:\n"
                                L" `-FunctionCallInstruction <line: 1, col: 15>\n"
                                L"  `-FunctionCall <line: 1, col: 15> functionName=print\n"
                                L"   `-ExponentExpression <line: 1, col: 21>\n"
                                L"    |-ExponentExpression <line: 1, col: 21>\n"
                                L"    ||-Literal <line: 1, col: 21> type=str value=4\n"
                                L"    |`-Literal <line: 1, col: 28> type=int value=2\n"
                                L"    `-Literal <line: 1, col: 33> type=str value=1\n"
    );
    doPriorityTest(
        L"\"4\" ! \"2\" @ \"2\"", L"FunctionDeclaration <line: 1, col: 1> source=<test>\n"
                                  L"`-Body:\n"
                                  L" `-FunctionCallInstruction <line: 1, col: 15>\n"
                                  L"  `-FunctionCall <line: 1, col: 15> functionName=print\n"
                                  L"   `-ConcatExpression <line: 1, col: 21>\n"
                                  L"    |-Literal <line: 1, col: 21> type=str value=4\n"
                                  L"    `-StringMultiplyExpression <line: 1, col: 27>\n"
                                  L"     |-Literal <line: 1, col: 27> type=str value=2\n"
                                  L"     `-Literal <line: 1, col: 33> type=str value=2\n"
    );
    doPriorityTest(
        L"3 === -\"-3\"", L"FunctionDeclaration <line: 1, col: 1> source=<test>\n"
                          L"`-Body:\n"
                          L" `-FunctionCallInstruction <line: 1, col: 15>\n"
                          L"  `-FunctionCall <line: 1, col: 15> functionName=print\n"
                          L"   `-IdenticalExpression <line: 1, col: 21>\n"
                          L"    |-Literal <line: 1, col: 21> type=int value=3\n"
                          L"    `-UnaryMinusExpression <line: 1, col: 27>\n"
                          L"     `-Literal <line: 1, col: 28> type=str value=-3\n"
    );
}

TEST_CASE("factorial example", "[Lexer+Parser]")
{
    std::wstring source = L"func factorial(int n) -> int {\n"
                          L"    if(n == 0 or n == 1) {\n"
                          L"        return 1;\n"
                          L"    }\n"
                          L"    else {\n"
                          L"        return n * factorial(n - 1);\n"
                          L"    }\n"
                          L"}\n"
                          L"\n"
                          L"func main() {\n"
                          L"    int a = factorial(3); # a === 6\n"
                          L"    int b = factorial(4); # b === 24\n"
                          L"}\n";
    std::set<std::wstring> expectedFunctions = {
        L"factorial(int): FunctionDeclaration <line: 1, col: 1> source=<test> returnType=int\n"
        L"|-Parameters:\n"
        L"|`-VariableDeclaration <line: 1, col: 16> type=int name=n mutable=false\n"
        L"`-Body:\n"
        L" `-IfStatement <line: 2, col: 5>\n"
        L"  |-SingleIfCase <line: 2, col: 5>\n"
        L"  ||-OrExpression <line: 2, col: 8>\n"
        L"  |||-EqualExpression <line: 2, col: 8>\n"
        L"  ||||-Variable <line: 2, col: 8> name=n\n"
        L"  |||`-Literal <line: 2, col: 13> type=int value=0\n"
        L"  ||`-EqualExpression <line: 2, col: 18>\n"
        L"  || |-Variable <line: 2, col: 18> name=n\n"
        L"  || `-Literal <line: 2, col: 23> type=int value=1\n"
        L"  |`-ReturnStatement <line: 3, col: 9>\n"
        L"  | `-Literal <line: 3, col: 16> type=int value=1\n"
        L"  `-ElseCase:\n"
        L"   `-ReturnStatement <line: 6, col: 9>\n"
        L"    `-MultiplyExpression <line: 6, col: 16>\n"
        L"     |-Variable <line: 6, col: 16> name=n\n"
        L"     `-FunctionCall <line: 6, col: 20> functionName=factorial\n"
        L"      `-MinusExpression <line: 6, col: 30>\n"
        L"       |-Variable <line: 6, col: 30> name=n\n"
        L"       `-Literal <line: 6, col: 34> type=int value=1\n",
        L"main: FunctionDeclaration <line: 10, col: 1> source=<test>\n"
        L"`-Body:\n"
        L" |-VariableDeclStatement <line: 11, col: 5>\n"
        L" ||-VariableDeclaration <line: 11, col: 5> type=int name=a mutable=false\n"
        L" |`-FunctionCall <line: 11, col: 13> functionName=factorial\n"
        L" | `-Literal <line: 11, col: 23> type=int value=3\n"
        L" `-VariableDeclStatement <line: 12, col: 5>\n"
        L"  |-VariableDeclaration <line: 12, col: 5> type=int name=b mutable=false\n"
        L"  `-FunctionCall <line: 12, col: 13> functionName=factorial\n"
        L"   `-Literal <line: 12, col: 23> type=int value=4\n"
    };
    checkLexingAndParsing(source, {}, {}, expectedFunctions);
}

TEST_CASE("typical errors", "[Lexer+Parser]")
{
    std::wstring source = L"func main() {\n"
                          L"    int a = 4;\n"
                          L"    if(a = 4) {\n"
                          L"        print(\"message\");\n"
                          L"    }\n"
                          L"}\n";
    REQUIRE_THROWS_AS(getTree(source), SyntaxError); // = instead of ==

    source = L"func main() {\n"
             L"    int a = 4;\n"
             L"    if(a == 4) {\n"
             L"        print(\"message\");\n"
             L"    }\n";
    REQUIRE_THROWS_AS(getTree(source), SyntaxError); // no closing brace

    source = L"func main() {\n"
             L"    int a = 4\n"
             L"    if(a == 4) {\n"
             L"        print(\"message\");\n"
             L"    }\n"
             L"}\n";
    REQUIRE_THROWS_AS(getTree(source), SyntaxError); // no semicolon
}
