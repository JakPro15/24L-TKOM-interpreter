#include "checkNodeContainer.hpp"
#include "commentDiscarder.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "parserExceptions.hpp"
#include "printingVisitor.hpp"
#include "semanticAnalysis.hpp"
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
    Program program = parser.parseProgram();
    doSemanticAnalysis(program);
    return program;
}

void checkProcessing(
    const std::wstring &source, const std::set<std::wstring> &expectedStructs,
    const std::set<std::wstring> &expectedVariants, const std::set<std::wstring> &expectedFunctions
)
{
    Program tree = getTree(source);

    checkNodeContainer(tree.structs, expectedStructs);
    checkNodeContainer(tree.variants, expectedVariants);
    checkNodeContainer(tree.functions, expectedFunctions);
}

std::wstring wrapInMain(const std::wstring &source)
{
    return L"func main() {\n" + source + L"}";
}
}

TEST_CASE("variable declarations", "[Lexer+Parser+SemanticAnalyzer]")
{
    std::wstring source = wrapInMain(L"int a = 20;\n"
                                     L"a = 21;\n");
    REQUIRE_THROWS(getTree(source));

    source = wrapInMain(L"int$ a = 20;\n"
                        L"a = 21;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 2, col: 1>\n"
         L" ||-VariableDeclaration <line: 2, col: 1> type=int name=a mutable=true\n"
         L" |`-Literal <line: 2, col: 10> type=int value=20\n"
         L" `-AssignmentStatement <line: 3, col: 1>\n"
         L"  |-Assignable <line: 3, col: 1> right=a\n"
         L"  `-Literal <line: 3, col: 5> type=int value=21\n"}
    );

    source = wrapInMain(L"int a = 20;\n"
                        L"a + 2 = 3;\n");
    REQUIRE_THROWS(getTree(source));
}

TEST_CASE("builtin types casts", "[Lexer+Parser+SemanticAnalyzer]")
{
    std::wstring source = wrapInMain(L"float a = \" -42\\r\\n\";\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=float name=a mutable=false\n"
         L"  `-CastExpression <line: 2, col: 11> targetType=float\n"
         L"   `-Literal <line: 2, col: 11> type=str value= -42\r\n\n"}
    );
    source = wrapInMain(L"int b = -4.5;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=b mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-UnaryMinusExpression <line: 2, col: 9>\n"
         L"    `-Literal <line: 2, col: 10> type=float value=4.5\n"}
    );
    source = wrapInMain(L"str c = -42;");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=str name=c mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=str\n"
         L"   `-UnaryMinusExpression <line: 2, col: 9>\n"
         L"    `-Literal <line: 2, col: 10> type=int value=42\n"}
    );
    source = wrapInMain(L"bool d = \"false\";");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=bool name=d mutable=false\n"
         L"  `-CastExpression <line: 2, col: 10> targetType=bool\n"
         L"   `-Literal <line: 2, col: 10> type=str value=false\n"}
    );
    source = wrapInMain(L"int e = \"4.2\";");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=e mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-Literal <line: 2, col: 9> type=str value=4.2\n"}
    );
}
