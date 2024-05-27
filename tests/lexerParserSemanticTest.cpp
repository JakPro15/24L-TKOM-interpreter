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
    program.add(
        {FunctionIdentification(L"print", {{Type::Builtin::STR}}),
         BuiltinFunctionDeclaration(
             Position{0, 0}, L"<builtins>",
             std::vector<VariableDeclaration>{VariableDeclaration{{0, 0}, {Type::Builtin::STR}, L"value", false}},
             std::nullopt,
             std::function<std::optional<Object>(std::vector<Object>)>([](std::vector<Object>) { return Object(); })
         )}
    );
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
    std::set<std::wstring> functionWithDummyBuiltins = expectedFunctions;
    functionWithDummyBuiltins.insert(L"print(str): BuiltinFunctionDeclaration <line: 0, col: 0> source=<builtins>\n"
                                     L"`-Parameters:\n"
                                     L" `-VariableDeclaration <line: 0, col: 0> type=str name=value mutable=false\n");
    checkNodeContainer(tree.functions, functionWithDummyBuiltins);
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
    std::wstring printedVariant = L"Variant: VariantDeclaration <line: 1, col: 1> source=<test>\n"
                                  L"|-Field <line: 2, col: 5> type=int name=a\n"
                                  L"|-Field <line: 3, col: 5> type=str name=b\n"
                                  L"`-Field <line: 4, col: 5> type=bool name=c\n";
    std::wstring source = variantSource + L"func main() {\n"
                                          L"    Variant$ a = 2; # a.a === 2\n"
                                          L"    a.b = \"string\";\n"
                                          L"    a.c = 3.4;      # domyślna konwersja na bool\n"
                                          L"    a = true;       # a is bool\n"
                                          L"}\n";
    checkProcessing(
        source, {}, {printedVariant},
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

    source = variantSource + L"func f(int i) -> int { return 1; }\n"
                             L"func f(Variant v) -> int { return 2; }\n"
                             L"\n"
                             L"func main() {\n"
                             L"    print(f(Variant(3))); # 2\n"
                             L"}\n";
    checkProcessing(
        source, {}, {printedVariant},
        {L"f(int): FunctionDeclaration <line: 6, col: 1> source=<test> returnType=int\n"
         L"|-Parameters:\n"
         L"|`-VariableDeclaration <line: 6, col: 8> type=int name=i mutable=false\n"
         L"`-Body:\n"
         L" `-ReturnStatement <line: 6, col: 24>\n"
         L"  `-Literal <line: 6, col: 31> type=int value=1\n",
         L"f(Variant): FunctionDeclaration <line: 7, col: 1> source=<test> returnType=int\n"
         L"|-Parameters:\n"
         L"|`-VariableDeclaration <line: 7, col: 8> type=Variant name=v mutable=false\n"
         L"`-Body:\n"
         L" `-ReturnStatement <line: 7, col: 28>\n"
         L"  `-Literal <line: 7, col: 35> type=int value=2\n",
         L"main: FunctionDeclaration <line: 9, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" `-FunctionCallInstruction <line: 10, col: 5>\n"
         L"  `-FunctionCall <line: 10, col: 5> functionName=print\n"
         L"   `-CastExpression <line: 10, col: 11> targetType=str\n"
         L"    `-FunctionCall <line: 10, col: 11> functionName=f\n"
         L"     `-CastExpression <line: 10, col: 13> targetType=Variant\n"
         L"      `-Literal <line: 10, col: 21> type=int value=3\n"}
    );

    source = variantSource +
             L"func main() {\n"
             L"    Variant$ a = 2; # a.a === 2\n"
             L"    bool b = a.c;   # błąd - dostęp do wartości rekordu wariantowego przez '.' poza 'if' z deklaracją\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = variantSource + L"func main() {\n"
                             L"    Variant$ a = 2; # a.a === 2\n"
                             L"    if(bool b = a) {}\n"
                             L"    if(bool b = a.c) {}\n"
                             L"}\n";
    checkProcessing(
        source, {}, {printedVariant},
        {L"main: FunctionDeclaration <line: 6, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 7, col: 5>\n"
         L" ||-VariableDeclaration <line: 7, col: 5> type=Variant name=a mutable=true\n"
         L" |`-CastExpression <line: 7, col: 18> targetType=Variant\n"
         L" | `-Literal <line: 7, col: 18> type=int value=2\n"
         L" |-IfStatement <line: 8, col: 5>\n"
         L" |`-SingleIfCase <line: 8, col: 5>\n"
         L" | `-VariableDeclStatement <line: 8, col: 8>\n"
         L" |  |-VariableDeclaration <line: 8, col: 8> type=bool name=b mutable=false\n"
         L" |  `-Variable <line: 8, col: 17> name=a\n"
         L" `-IfStatement <line: 9, col: 5>\n"
         L"  `-SingleIfCase <line: 9, col: 5>\n"
         L"   `-VariableDeclStatement <line: 9, col: 8>\n"
         L"    |-VariableDeclaration <line: 9, col: 8> type=bool name=b mutable=false\n"
         L"    `-Variable <line: 9, col: 17> name=a\n"}
    );

    source = variantSource + L"func main() {\n"
                             L"    Variant$ a = 2; # a.a === 2\n"
                             L"    if(int$ value = a) {\n"
                             L"        value = 3;\n"
                             L"    }\n"
                             L"}\n";
    checkProcessing(
        source, {}, {printedVariant},
        {L"main: FunctionDeclaration <line: 6, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 7, col: 5>\n"
         L" ||-VariableDeclaration <line: 7, col: 5> type=Variant name=a mutable=true\n"
         L" |`-CastExpression <line: 7, col: 18> targetType=Variant\n"
         L" | `-Literal <line: 7, col: 18> type=int value=2\n"
         L" `-IfStatement <line: 8, col: 5>\n"
         L"  `-SingleIfCase <line: 8, col: 5>\n"
         L"   |-VariableDeclStatement <line: 8, col: 8>\n"
         L"   ||-VariableDeclaration <line: 8, col: 8> type=int name=value mutable=true\n"
         L"   |`-Variable <line: 8, col: 21> name=a\n"
         L"   `-AssignmentStatement <line: 9, col: 9>\n"
         L"    |-Assignable <line: 9, col: 9> right=value\n"
         L"    `-Literal <line: 9, col: 17> type=int value=3\n"}
    );

    source = variantSource + L"func main() {\n"
                             L"    Variant$ a = 2; # a.a === 2\n"
                             L"    if(int$ value = a) {}\n"
                             L"    value = 3;\n"
                             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"variant Variant1 {\n"
             L"    int a;\n"
             L"    int b; # błąd - dwa pola rekordu wariantowego o tym samym typie\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"variant Variant1 {\n"
             L"    int$ a; # błąd - pole rekordu wariantowego oznaczone jako mutowalne\n"
             L"    str b;\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"variant Variant {\n"
             L"    int a;\n"
             L"    str b;\n"
             L"}\n"
             L"struct Struct {\n"
             L"    int a;\n"
             L"    str b;\n"
             L"}\n"
             L"variant StructOrVariant {\n"
             L"    Struct a;\n"
             L"    Variant b;\n"
             L"}\n";
    checkProcessing(
        source,
        {L"Struct: StructDeclaration <line: 5, col: 1> source=<test>\n"
         L"|-Field <line: 6, col: 5> type=int name=a\n"
         L"`-Field <line: 7, col: 5> type=str name=b\n"},
        {L"Variant: VariantDeclaration <line: 1, col: 1> source=<test>\n"
         L"|-Field <line: 2, col: 5> type=int name=a\n"
         L"`-Field <line: 3, col: 5> type=str name=b\n",
         L"StructOrVariant: VariantDeclaration <line: 9, col: 1> source=<test>\n"
         L"|-Field <line: 10, col: 5> type=Struct name=a\n"
         L"`-Field <line: 11, col: 5> type=Variant name=b\n"},
        {}
    );

    source = L"variant Wrong {\n"
             L"    int a;\n"
             L"    Wrong b; # błąd - rekord wariantowy zawiera sam siebie\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"struct Struct {\n"
             L"    int a;\n"
             L"    StructOrInt b;\n"
             L"}\n"
             L"variant StructOrInt {\n"
             L"    Struct a; # błąd - rekord wariantowy zawiera sam siebie\n"
             L"    int b;\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));
}

TEST_CASE("structures", "[Lexer+Parser+SemanticAnalyzer]")
{
    std::wstring structsSource = L"struct OtherStruct {\n"
                                 L"    int a;\n"
                                 L"    str b;\n"
                                 L"}\n"
                                 L"struct MyStruct {\n"
                                 L"    int a;\n"
                                 L"    float b;\n"
                                 L"    OtherStruct c;\n"
                                 L"    bool d;\n"
                                 L"}\n";
    std::set<std::wstring> printedStructs = {
        L"OtherStruct: StructDeclaration <line: 1, col: 1> source=<test>\n"
        L"|-Field <line: 2, col: 5> type=int name=a\n"
        L"`-Field <line: 3, col: 5> type=str name=b\n",
        L"MyStruct: StructDeclaration <line: 5, col: 1> source=<test>\n"
        L"|-Field <line: 6, col: 5> type=int name=a\n"
        L"|-Field <line: 7, col: 5> type=float name=b\n"
        L"|-Field <line: 8, col: 5> type=OtherStruct name=c\n"
        L"`-Field <line: 9, col: 5> type=bool name=d\n"
    };
    checkProcessing(structsSource, printedStructs, {}, {});

    std::wstring source = L"struct Struct1 {\n"
                          L"    int$ a; # błąd - pole struktury oznaczone jako mutowalne\n"
                          L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"struct Struct1 {\n"
             L"    Struct1 a; # błąd - struktura zawiera samą siebie\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = L"struct Struct1 {\n"
             L"    Struct2 a;\n"
             L"}\n"
             L"struct Struct2 {\n"
             L"    Struct1 a; # błąd - struktura zawiera samą siebie\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));

    source = structsSource + L"func function_taking_MyStruct(MyStruct a) {}\n"
                             L"func main() {\n"
                             L"    MyStruct a = {2, 4.3, {4, \"a\"}, true};\n"
                             L"    int$ b = MyStruct({2, 4.3, {4, \"a\"}, true}).a;\n"
                             L"    function_taking_MyStruct(MyStruct({2, 4.3, {4, \"a\"}, true}));\n"
                             L"    b = a.c.a; # b === 4\n"
                             L"}\n";
    checkProcessing(
        source, printedStructs, {},
        {L"function_taking_MyStruct(MyStruct): FunctionDeclaration <line: 11, col: 1> source=<test>\n"
         L"`-Parameters:\n"
         L" `-VariableDeclaration <line: 11, col: 31> type=MyStruct name=a mutable=false\n",
         L"main: FunctionDeclaration <line: 12, col: 1> source=<test>\n"
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 13, col: 5>\n"
         L" ||-VariableDeclaration <line: 13, col: 5> type=MyStruct name=a mutable=false\n"
         L" |`-StructExpression <line: 13, col: 18> structType=MyStruct\n"
         L" | |-Literal <line: 13, col: 19> type=int value=2\n"
         L" | |-Literal <line: 13, col: 22> type=float value=4.3\n"
         L" | |-StructExpression <line: 13, col: 27> structType=OtherStruct\n"
         L" | ||-Literal <line: 13, col: 28> type=int value=4\n"
         L" | |`-Literal <line: 13, col: 31> type=str value=a\n"
         L" | `-Literal <line: 13, col: 37> type=bool value=true\n"
         L" |-VariableDeclStatement <line: 14, col: 5>\n"
         L" ||-VariableDeclaration <line: 14, col: 5> type=int name=b mutable=true\n"
         L" |`-DotExpression <line: 14, col: 14> field=a\n"
         L" | `-StructExpression <line: 14, col: 23> structType=MyStruct\n"
         L" |  |-Literal <line: 14, col: 24> type=int value=2\n"
         L" |  |-Literal <line: 14, col: 27> type=float value=4.3\n"
         L" |  |-StructExpression <line: 14, col: 32> structType=OtherStruct\n"
         L" |  ||-Literal <line: 14, col: 33> type=int value=4\n"
         L" |  |`-Literal <line: 14, col: 36> type=str value=a\n"
         L" |  `-Literal <line: 14, col: 42> type=bool value=true\n"
         L" |-FunctionCallInstruction <line: 15, col: 5>\n"
         L" |`-FunctionCall <line: 15, col: 5> functionName=function_taking_MyStruct\n"
         L" | `-StructExpression <line: 15, col: 39> structType=MyStruct\n"
         L" |  |-Literal <line: 15, col: 40> type=int value=2\n"
         L" |  |-Literal <line: 15, col: 43> type=float value=4.3\n"
         L" |  |-StructExpression <line: 15, col: 48> structType=OtherStruct\n"
         L" |  ||-Literal <line: 15, col: 49> type=int value=4\n"
         L" |  |`-Literal <line: 15, col: 52> type=str value=a\n"
         L" |  `-Literal <line: 15, col: 58> type=bool value=true\n"
         L" `-AssignmentStatement <line: 16, col: 5>\n"
         L"  |-Assignable <line: 16, col: 5> right=b\n"
         L"  `-DotExpression <line: 16, col: 9> field=a\n"
         L"   `-DotExpression <line: 16, col: 9> field=c\n"
         L"    `-Variable <line: 16, col: 9> name=a\n"}
    );

    source = L"struct S1 {\n"
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
             L"    S2 s = {{2, 3}, 2};\n"
             L"    # konieczne jest podanie typu S1, ponieważ jest wewnątrz rekordu wariantowego\n"
             L"}\n";
    REQUIRE_THROWS(getTree(source));
}
