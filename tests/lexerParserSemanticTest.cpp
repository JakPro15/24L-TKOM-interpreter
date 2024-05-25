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

TEST_CASE("operators casts", "[Lexer+Parser+SemanticAnalyzer]")
{
    std::wstring source = wrapInMain(L"bool a = (2 + 2.3) is float;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=bool name=a mutable=false\n"
         L"  `-IsExpression <line: 2, col: 10> right=float\n"
         L"   `-PlusExpression <line: 2, col: 11>\n"
         L"    |-CastExpression <line: 2, col: 11> targetType=float\n"
         L"    |`-Literal <line: 2, col: 11> type=int value=2\n"
         L"    `-Literal <line: 2, col: 15> type=float value=2.3\n"}
    );
    source = wrapInMain(L"bool b = (2.3 - 2) is float;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=bool name=b mutable=false\n"
         L"  `-IsExpression <line: 2, col: 10> right=float\n"
         L"   `-MinusExpression <line: 2, col: 11>\n"
         L"    |-Literal <line: 2, col: 11> type=float value=2.3\n"
         L"    `-CastExpression <line: 2, col: 17> targetType=float\n"
         L"     `-Literal <line: 2, col: 17> type=int value=2\n"}
    );
    source = wrapInMain(L"bool c = (2 * 3) is int;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=bool name=c mutable=false\n"
         L"  `-IsExpression <line: 2, col: 10> right=int\n"
         L"   `-MultiplyExpression <line: 2, col: 11>\n"
         L"    |-Literal <line: 2, col: 11> type=int value=2\n"
         L"    `-Literal <line: 2, col: 15> type=int value=3\n"}
    );
    source = wrapInMain(L"int a = 2 / 2 * 3;\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=a mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-MultiplyExpression <line: 2, col: 9>\n"
         L"    |-DivideExpression <line: 2, col: 9>\n"
         L"    ||-CastExpression <line: 2, col: 9> targetType=float\n"
         L"    ||`-Literal <line: 2, col: 9> type=int value=2\n"
         L"    |`-CastExpression <line: 2, col: 13> targetType=float\n"
         L"    | `-Literal <line: 2, col: 13> type=int value=2\n"
         L"    `-CastExpression <line: 2, col: 17> targetType=float\n"
         L"     `-Literal <line: 2, col: 17> type=int value=3\n"}
    );
    source = wrapInMain(L"int b = \"4\" ** 2 ** \"1\";\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=b mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-ExponentExpression <line: 2, col: 9>\n"
         L"    |-ExponentExpression <line: 2, col: 9>\n"
         L"    ||-CastExpression <line: 2, col: 9> targetType=float\n"
         L"    ||`-Literal <line: 2, col: 9> type=str value=4\n"
         L"    |`-CastExpression <line: 2, col: 16> targetType=float\n"
         L"    | `-Literal <line: 2, col: 16> type=int value=2\n"
         L"    `-CastExpression <line: 2, col: 21> targetType=float\n"
         L"     `-Literal <line: 2, col: 21> type=str value=1\n"}
    );
    source = wrapInMain(L"str c = \"4\" ! \"2\" @ \"2\";\n");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=str name=c mutable=false\n"
         L"  `-ConcatExpression <line: 2, col: 9>\n"
         L"   |-Literal <line: 2, col: 9> type=str value=4\n"
         L"   `-StringMultiplyExpression <line: 2, col: 15>\n"
         L"    |-Literal <line: 2, col: 15> type=str value=2\n"
         L"    `-CastExpression <line: 2, col: 21> targetType=int\n"
         L"     `-Literal <line: 2, col: 21> type=str value=2\n"}
    );
    source = wrapInMain(L"float e = 2 / 4.0;");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=float name=e mutable=false\n"
         L"  `-DivideExpression <line: 2, col: 11>\n"
         L"   |-CastExpression <line: 2, col: 11> targetType=float\n"
         L"   |`-Literal <line: 2, col: 11> type=int value=2\n"
         L"   `-Literal <line: 2, col: 15> type=float value=4\n"}
    );
    source = wrapInMain(L"bool f = 3 === -\"-3\";");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=bool name=f mutable=false\n"
         L"  `-IdenticalExpression <line: 2, col: 10>\n"
         L"   |-Literal <line: 2, col: 10> type=int value=3\n"
         L"   `-UnaryMinusExpression <line: 2, col: 16>\n"
         L"    `-CastExpression <line: 2, col: 17> targetType=float\n"
         L"     `-Literal <line: 2, col: 17> type=str value=-3\n"}
    );
    source = wrapInMain(L"int g = \"2345\"[\"1\"];");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=g mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-SubscriptExpression <line: 2, col: 9>\n"
         L"    |-Literal <line: 2, col: 9> type=str value=2345\n"
         L"    `-CastExpression <line: 2, col: 16> targetType=int\n"
         L"     `-Literal <line: 2, col: 16> type=str value=1\n"}
    );
    source = wrapInMain(L"int h = \"abc\"[\"2.1\"];");
    checkProcessing(
        source, {}, {},
        {L"main: FunctionDeclaration <line: 1, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-VariableDeclStatement <line: 2, col: 1>\n"
         L"  |-VariableDeclaration <line: 2, col: 1> type=int name=h mutable=false\n"
         L"  `-CastExpression <line: 2, col: 9> targetType=int\n"
         L"   `-SubscriptExpression <line: 2, col: 9>\n"
         L"    |-Literal <line: 2, col: 9> type=str value=abc\n"
         L"    `-CastExpression <line: 2, col: 15> targetType=int\n"
         L"     `-Literal <line: 2, col: 15> type=str value=2.1\n"}
    );
    source = wrapInMain(L"bool a = 2 < 3 < 4;\n");
    REQUIRE_THROWS(getTree(source));
}

TEST_CASE("variants", "[Lexer+Parser+SemanticAnalyzer]")
{
    std::wstring variantSource = L"variant Variant {\n"
                                 L"    int a;\n"
                                 L"    str b;\n"
                                 L"    bool c;\n"
                                 L"}\n";
    std::wstring source = variantSource + L"func main() {\n"
                                          L"    Variant$ a = 2; # a.a === 2\n"
                                          L"    a.b = \"string\";\n"
                                          L"    a.c = 3.4;      # domyślna konwersja na bool\n"
                                          L"    a = true;       # a is bool\n"
                                          L"}\n";
    checkProcessing(
        source, {},
        {L"Variant: VariantDeclaration <line: 1, col: 1> source=<test>\n"
         L"|-Field <line: 2, col: 5> type=int name=a\n"
         L"|-Field <line: 3, col: 5> type=str name=b\n"
         L"`-Field <line: 4, col: 5> type=bool name=c\n"},
        {L"main: FunctionDeclaration <line: 6, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 7, col: 5>\n"
         L" ||-VariableDeclaration <line: 7, col: 5> type=Variant name=a mutable=true\n"
         L" |`-CastExpression <line: 7, col: 18> targetType=Variant\n"
         L" | `-Literal <line: 7, col: 18> type=int value=2\n"
         L" |-AssignmentStatement <line: 8, col: 5>\n"
         L" ||-Assignable <line: 8, col: 5> right=b\n"
         L" ||`-Assignable <line: 8, col: 5> right=a\n"
         L" |`-Literal <line: 8, col: 11> type=str value=string\n"
         L" |-AssignmentStatement <line: 9, col: 5>\n"
         L" ||-Assignable <line: 9, col: 5> right=c\n"
         L" ||`-Assignable <line: 9, col: 5> right=a\n"
         L" |`-CastExpression <line: 9, col: 11> targetType=bool\n"
         L" | `-Literal <line: 9, col: 11> type=float value=3.4\n"
         L" `-AssignmentStatement <line: 10, col: 5>\n"
         L"  |-Assignable <line: 10, col: 5> right=a\n"
         L"  `-CastExpression <line: 10, col: 9> targetType=Variant\n"
         L"   `-Literal <line: 10, col: 9> type=bool value=true\n"}
    );

    source = variantSource + L"func main() {\n"
                             L"    Variant$ a = 2; # a.a === 2\n"
                             L"    a = 3.4;        # błąd - nieprawidłowy typ przypisany do rekordu wariantowego\n"
                             L"}\n";
    REQUIRE_THROWS(getTree(source));

    // source = variantSource + L"func f(int i) -> int { return 1; }\n"
    //                          L"func f(Variant v) -> int { return 2; }\n"
    //                          L"\n"
    //                          L"func main() {\n"
    //                          L"    print(f(Variant(3))); # 2\n"
    //                          L"}\n";
    // checkProcessing(
    //     source, {},
    //     {L"Variant: VariantDeclaration <line: 1, col: 1> source=<test>\n"
    //      L"|-Field <line: 2, col: 5> type=int name=a\n"
    //      L"|-Field <line: 3, col: 5> type=str name=b\n"
    //      L"`-Field <line: 4, col: 5> type=bool name=c\n"},
    //     {L"f(int): FunctionDeclaration <line: 6, col: 1> source=<test> returnType=int\n"
    //      L"|-Parameters:\n"
    //      L"|`-VariableDeclaration <line: 6, col: 8> type=int name=i mutable=false\n"
    //      L"`-Body:\n"
    //      L" `-ReturnStatement <line: 6, col: 24>\n"
    //      L"  `-Literal <line: 6, col: 31> type=int value=1\n",
    //      L"f(Variant): FunctionDeclaration <line: 7, col: 1> source=<test> returnType=int\n"
    //      L"|-Parameters:\n"
    //      L"|`-VariableDeclaration <line: 7, col: 8> type=int name=i mutable=false\n"
    //      L"`-Body:\n"
    //      L" `-ReturnStatement <line: 7, col: 28>\n"
    //      L"  `-Literal <line: 7, col: 35> type=int value=2\n",
    //      L"main: FunctionDeclaration <line: 9, col: 1> source=<test>\n"
    //      L"`-Body:\n"
    //      L" |-FunctionCallInstruction <line: 10, col: 5>\n"}
    // );
}
