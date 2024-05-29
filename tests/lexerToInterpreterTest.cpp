#include "commentDiscarder.hpp"
#include "helpers.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "lexerExceptions.hpp"
#include "parser.hpp"
#include "parserExceptions.hpp"
#include "runtimeExceptions.hpp"
#include "semanticExceptions.hpp"
#include "streamReader.hpp"

namespace {
std::wstring interpret(const std::wstring &sourceCode)
{
    std::wstringstream sourceStream(sourceCode);
    StreamReader reader(sourceStream, L"<test>");
    Lexer lexer(reader);
    CommentDiscarder commentDiscarder(lexer);
    Parser parser(commentDiscarder);
    Program program = parser.parseProgram();
    std::wstringstream inputStream, outputStream;
    std::vector<std::wstring> sourceFiles = {L"<test>"};
    Interpreter interpreter(sourceFiles, {}, inputStream, outputStream, [](const std::wstring &) -> Program {
        throw std::runtime_error("No files should be included in these tests");
    });
    interpreter.visit(program);
    return outputStream.str();
}
}

TEST_CASE("variable declarations", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(
        interpret(wrapInMain(L"int a = 20;\n"
                             L"print(a);")) == L"20"
    );
    REQUIRE_THROWS_AS(
        interpret(wrapInMain(L"int a = 20;\n"
                             L"a = 21;")),
        ImmutableError
    );
    REQUIRE(
        interpret(wrapInMain(L"int$ a = 20;\n"
                             L"a = 21;\n"
                             L"print(a);")) == L"21"
    );
    REQUIRE_THROWS_AS(
        interpret(wrapInMain(L"int a = 20;\n"
                             L"a + 2 = 3;")),
        SyntaxError
    );
}

TEST_CASE("string handling", "[Lexer+Parser+Interpreter]")
{
    std::wstring binaryOutput = interpret(wrapInMain(L"str a = \"abc\\t\\r\\n\\\"\\\\\\x00\\xb7\";\n"
                                                     L"print(a);"));
    std::wstring expected;
    expected.resize(10);
    std::memcpy(
        reinterpret_cast<char *>(&expected[0]), reinterpret_cast<const char *>(L"abc\t\r\n\"\\\x00\xb7"),
        10 * sizeof(wchar_t)
    );
    REQUIRE(binaryOutput == expected);

    REQUIRE_THROWS_AS(interpret(wrapInMain(L"str b = \"\\k\";")), UnknownEscapeSequenceError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"str c = \"\\xg5\";")), InvalidHexCharError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"str d = \"a01\"g5\";")), UnterminatedStringError);
}

TEST_CASE("type conversions", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(interpret(wrapInMain(L"float a = \" -42\\r\\n\"; print(a === -42.0);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int b = -4.5;             print(b === -5);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"str c = -42;              print(c === \"-42\");")) == L"true");
    REQUIRE(interpret(wrapInMain(L"bool d = \"false\";       print(d === true);")) == L"true");
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int e = \"4.2\";")), CastImpossibleError);
    REQUIRE(interpret(wrapInMain(L"int a = int(\"42\");      print(a === 42);")) == L"true");
}

TEST_CASE("operators", "[Lexer+Parser+Interpreter]")
{
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"str a = \"abc\"[-1];")), OperatorArgumentError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"str a = \"abc\"[3];")), OperatorArgumentError);

    REQUIRE_THROWS_AS(interpret(wrapInMain(L"float a = 2.5 / 0.0;")), ZeroDivisionError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int b = 2 // 0;")), ZeroDivisionError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int c = 2 % 0;")), ZeroDivisionError);

    REQUIRE(interpret(wrapInMain(L"int a = -22 // 5;  print(a === -5);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int b = -22 % 5;   print(b === 3);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int c = -22 // -5; print(c === 4);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int d = -22 % -5;  print(d === -2);")) == L"true");

    REQUIRE_THROWS_AS(interpret(wrapInMain(L"float a = -2 ** 5;")), OperatorArgumentError);
    REQUIRE(interpret(wrapInMain(L"float b = 0 ** 5;  print(b === 0.0);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"float c = 2 ** 5;  print(c === 32.0);")) == L"true");

    REQUIRE(interpret(wrapInMain(L"print(2 == 2.4);")) == L"false");
    REQUIRE(interpret(wrapInMain(L"print(\"2.4\" == 2.4);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"print(true != 1);")) == L"false");
    REQUIRE(interpret(wrapInMain(L"print(\"1\" != true);")) == L"true");

    REQUIRE(interpret(wrapInMain(L"print(\"2.4\" === 2.4);")) == L"false");
    REQUIRE(interpret(wrapInMain(L"print(true !== 1);")) == L"true");

    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = 2147483647 + 1;")), IntegerRangeError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = -2147483647 - 2;")), IntegerRangeError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = 1073741824 * 2;")), IntegerRangeError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = -(-2147483647 - 1);")), IntegerRangeError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = (-2147483647 - 1) // -1;")), IntegerRangeError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int a = 1e100;")), CastImpossibleError);

    REQUIRE(
        interpret(L"struct S {int a; str b;}\n"
                  L"func main() {\n"
                  L"    print(S({1, \"a\"}) == S({1, \"a\"})); # true\n"
                  L"}") == L"true"
    );
    REQUIRE(
        interpret(L"variant V {int a; str b;}\n"
                  L"struct S {int a; V b;}\n"
                  L"func main() {\n"
                  L"    print(S({1, \"2\"}) != S({1, 2}));     # true\n"
                  L"    print(S({1, \"2\"}) == S({1, \"2\"})); # true\n"
                  L"}") == L"truetrue"
    );
    REQUIRE(
        interpret(L"struct S {int a; str b;}\n"
                  L"variant V {int a; str b; S c;}\n"
                  L"func main() {\n"
                  L"    print(V(2) == V(\"2\"));         # true\n"
                  L"    print(V(2) != V(S({2, \"2\"}))); # true\n"
                  L"}") == L"truetrue"
    );

    REQUIRE(interpret(wrapInMain(L"bool a = (2 + 2.3) is float; print(a === true);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"bool b = (2.3 - 2) is float; print(b === true);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"bool c = (2 * 3) is int;     print(c === true);")) == L"true");

    REQUIRE(interpret(wrapInMain(L"int a = 2 / 2 * 3;             print(a === 3);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int b = \"4\" ** 2 ** \"1\";   print(b === 16);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"str c = \"4\" ! \"2\" @ \"2\"; print(c === \"422\");")) == L"true");
    REQUIRE(interpret(wrapInMain(L"float e = 2 / 4.0;             print(e === 0.5);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"bool f = 3 === -\"-3\";        print(f === false);")) == L"true");
    REQUIRE(interpret(wrapInMain(L"int g = \"2345\"[\"1\"];       print(g === 3);")) == L"true");
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"int h = \"abc\"[\"2.1\"];")), CastImpossibleError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"bool a = 2 < 3 < 4;")), SyntaxError);
}

TEST_CASE("variants", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"func main() {\n"
                  L"    Variant$ a = 2;   if(int val = a.a) {println(val);}\n"
                  L"    a.b = \"string\"; if(str val = a.b) {println(val);}\n"
                  L"    a.c = 3.4;        if(bool val = a)  {println(val);}\n"
                  L"    a = true;         print(a is bool);\n"
                  L"}") == L"2\nstring\ntrue\ntrue"
    );
    REQUIRE_THROWS_AS(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"func main() {\n"
                  L"    Variant$ a = 2;\n"
                  L"    a = 3.4; # błąd - nieprawidłowy typ przypisany do rekordu wariantowego\n"
                  L"}"),
        InvalidCastError
    );
    REQUIRE(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"func f(int i) -> int { return 1; }\n"
                  L"func f(Variant v) -> int { return 2; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    print(f(Variant(3))); # 2\n"
                  L"}") == L"2"
    );
    REQUIRE_THROWS_AS(
        interpret(
            L"variant Variant {int a; str b; bool c;}\n"
            L"func main() {\n"
            L"    Variant$ a = 2;\n"
            L"    bool b = a.c; # błąd - dostęp do wartości rekordu wariantowego przez '.' poza 'if' z deklaracją\n"
            L"}"
        ),
        FieldAccessError
    );
    REQUIRE(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"\n"
                  L"func main() {\n"
                  L"    Variant$ a = 2;\n"
                  L"    if(a is bool) {\n"
                  L"        print(\"bool\");\n"
                  L"    }\n"
                  L"    else {\n"
                  L"        print(\"not bool\");\n"
                  L"    }\n"
                  L"}") == L"not bool"
    );
    REQUIRE(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"func printType(Variant a) {\n"
                  L"    if(int value = a) {\n"
                  L"        print(\"int\");\n"
                  L"    }\n"
                  L"    elif(str value = a.b) {\n"
                  L"        print(\"str\");\n"
                  L"    }\n"
                  L"    elif(bool value = a) {\n"
                  L"        print(\"bool\");\n"
                  L"    }\n"
                  L"}\n"
                  L"func main() {\n"
                  L"    printType(Variant(2));\n"
                  L"    printType(Variant(\"2\"));\n"
                  L"    printType(Variant(true));\n"
                  L"}") == L"intstrbool"
    );
    REQUIRE(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"\n"
                  L"func main() {\n"
                  L"    Variant a = 2;\n"
                  L"    if(int$ value = a) {\n"
                  L"        value = 3;\n"
                  L"    }\n"
                  L"    if(int val = a) { print(val); }\n"
                  L"}") == L"2"
    );
    REQUIRE_THROWS_AS(
        interpret(L"variant Variant {int a; str b; bool c;}\n"
                  L"func main() {\n"
                  L"    a.a = 2;\n"
                  L"    if(int$ value = a) {}\n"
                  L"    value = 3; # błąd - nieznana zmienna value\n"
                  L"}"),
        UnknownVariableError
    );
    REQUIRE_THROWS_AS(interpret(L"variant Variant1 {int a; int b;}"), FieldTypeCollisionError);
    REQUIRE_THROWS_AS(interpret(L"variant Variant2 {int$ a; str b;}"), SyntaxError);
    REQUIRE(
        interpret(L"variant Variant {\n"
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
                  L"}\n"
                  L"func main() {\n"
                  L"    StructOrVariant v = Struct({2, \"1\"});\n"
                  L"    if(Struct s = v) {print(s.a ! s.b);}\n"
                  L"}") == L"21"
    );
    REQUIRE_THROWS_AS(interpret(L"variant Wrong {int a; Wrong b;}"), FieldTypeRecursionError);
    REQUIRE_THROWS_AS(
        interpret(L"struct Struct {int a; StructOrInt b;}\n"
                  L"variant StructOrInt {Struct a; int b;}"),
        FieldTypeRecursionError
    );
    REQUIRE(
        interpret(L"variant V {int a; str b; bool c;}\n"
                  L"func f(int v) -> int { return 1; }\n"
                  L"func f(str v) -> int { return 2; }\n"
                  L"func f(bool v) -> int { return 3; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    V vart1 = 2;\n"
                  L"    V vart2 = \"string\";\n"
                  L"    print(f(vart1)); # 1\n"
                  L"    print(f(vart2)); # 2\n"
                  L"}\n") == L"12"
    );
    REQUIRE_THROWS_AS(
        interpret(L"variant V {int a; str b; bool c;}\n"
                  L"func f(int v) -> int { return 1; }\n"
                  L"func f(str v) -> int { return 2; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    V vart1 = 2;\n"
                  L"    print(f(vart1)); # błąd - niezdefiniowane funkcje dla wszystkich pól rekordu wariantowego\n"
                  L"}\n"),
        InvalidFunctionCallError
    );
    REQUIRE(
        interpret(L"variant V {int a; str b;}\n"
                  L"func f(int v) -> int { return 1; }\n"
                  L"func f(str v) -> int { return 2; }\n"
                  L"\n"
                  L"func f(V v) -> int { return 3; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    V vart1 = 2;\n"
                  L"    print(f(vart1)); # 3\n"
                  L"}\n") == L"3"
    );
    REQUIRE(
        interpret(L"variant V {int a; str b;}\n"
                  L"func f(int a, int v1, int v2) -> int { return 1; }\n"
                  L"func f(int a, str v1, int v2) -> int { return 2; }\n"
                  L"func f(int a, int v1, str v2) -> int { return 3; }\n"
                  L"func f(int a, str v1, str v2) -> int { return 4; }\n"
                  L"\n"
                  L"func f(V v) -> int { return 3; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    V vart1 = 2;\n"
                  L"    V vart2 = \"2\";\n"
                  L"    print(f(2.5, vart1, vart2)); # 3\n"
                  L"}\n") == L"3"
    );
    REQUIRE_THROWS_AS(
        interpret(L"variant V {int a; str b;}\n"
                  L"func f(int v) -> int { return 1; }\n"
                  L"func f(str v) -> str { return \"2\"; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    V vart1 = 2;\n"
                  L"    print(f(vart1)); # błąd - nieprawidłowe typy funkcji przeciążonych\n"
                  L"}\n"),
        InvalidFunctionCallError
    );
    REQUIRE_THROWS_AS(
        interpret(
            L"variant V {int a; str b;}\n"
            L"func f(int a, int v) -> int { return 1; }\n"
            L"func f(float a, str v) -> int { return 2; }\n"
            L"\n"
            L"func main() {\n"
            L"    V vart1 = 2;\n"
            L"    print(f(2, vart1)); # błąd - nieprawidłowe typy argumentu niewariantowego funkcji przeciążonych\n"
            L"}\n"
        ),
        InvalidFunctionCallError
    );
}

TEST_CASE("structs", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(
        interpret(L"struct OtherStruct {int a; str b;}\n"
                  L"struct MyStruct {int a; float b; OtherStruct c; bool d;}\n"
                  L"func main() {\n"
                  L"    MyStruct a = {2, 4.3, {4, \"a\"}, true};\n"
                  L"    int b = a.c.a; # b === 4\n"
                  L"    print(b);\n"
                  L"}\n") == L"4"
    );
    REQUIRE_THROWS_AS(interpret(L"struct Struct1 {int$ a;}"), SyntaxError);
    REQUIRE_THROWS_AS(interpret(L"struct Struct1 {Struct1 a;}"), FieldTypeRecursionError);
    REQUIRE_THROWS_AS(
        interpret(L"struct Struct1 {Struct2 a;}\n"
                  L"struct Struct2 {Struct1 a;}"),
        FieldTypeRecursionError
    );
    REQUIRE(
        interpret(L"struct OtherStruct {int a; str b;}\n"
                  L"struct MyStruct {int a; float b; OtherStruct c; bool d;}\n"
                  L"func function_taking_MyStruct(MyStruct s) {\n"
                  L"    print(s.d);\n"
                  L"}\n"
                  L"func main() {\n"
                  L"    int a = MyStruct({2, 4.3, {4, \"a\"}, true}).a;\n"
                  L"    println(a);\n"
                  L"    function_taking_MyStruct({2, 4.3, {4, \"a\"}, true});\n"
                  L"}\n") == L"2\ntrue"
    );
    REQUIRE(
        interpret(L"struct S1 {int a; int b;}\n"
                  L"variant V {S1 s; int i;}\n"
                  L"struct S2 {V v; int i;}\n"
                  L"func main() {\n"
                  L"    S2 s = {S1({2, 3}), 2};\n"
                  L"    # konieczne jest podanie typu S1, ponieważ jest wewnątrz rekordu wariantowego\n"
                  L"    if(S1 s1 = s.v) {print(s1.b);}\n"
                  L"}\n") == L"3"
    );
    REQUIRE_THROWS_AS(
        interpret(L"struct S1 {int a; int b;}\n"
                  L"variant V {S1 s; int i;}\n"
                  L"struct S2 {V v; int i;}\n"
                  L"func main() {\n"
                  L"    S2 s = {{2, 3}, 2};\n"
                  L"}\n"),
        InvalidCastError
    );
}

TEST_CASE("ifs", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(
        interpret(L"variant V {int a; str b;}\n"
                  L"func do_if(V v, int a) {\n"
                  L"    if(a == 3) {print(1);}\n"
                  L"    elif(int val = v) {print(2);}\n"
                  L"    elif(a == 2) {print(3);}\n"
                  L"    else {print(4);}\n"
                  L"}\n"
                  L"func main() {\n"
                  L"    do_if(2, 3);\n"
                  L"    do_if(3, 2);\n"
                  L"    do_if(\"2\", 2);\n"
                  L"    do_if(\"2\", 1);\n"
                  L"}\n") == L"1234"
    );
}

TEST_CASE("loops", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(
        interpret(wrapInMain(L"int$ a = 1;\n"
                             L"while(false) {\n"
                             L"    a = a + 1;\n"
                             L"}\n"
                             L"print(a);\n")) == L"1"
    );
    REQUIRE(
        interpret(wrapInMain(L"int$ a = 1;\n"
                             L"do {\n"
                             L"    a = a + 1;\n"
                             L"} while(false)\n"
                             L"print(a);\n")) == L"2"
    );

    REQUIRE(
        interpret(wrapInMain(L"int$ a = 0;\n"
                             L"int$ b = 0;\n"
                             L"while(a < 10) {\n"
                             L"    do {\n"
                             L"        b = b + 1;\n"
                             L"        if(a > 4) {\n"
                             L"            continue;\n"
                             L"        }\n"
                             L"        a = a + 1;\n"
                             L"    } while(b < 10)\n"
                             L"    if(b == 13) {\n"
                             L"        break;\n"
                             L"    }\n"
                             L"    a = a + 1;\n"
                             L"}\n"
                             L"println(\"a = \" ! a);\n"
                             L"println(\"b = \" ! b);\n")) == L"a = 8\nb = 13\n"
    );
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"continue;\n")), InvalidContinueError);
    REQUIRE_THROWS_AS(interpret(wrapInMain(L"break;\n")), InvalidBreakError);
}

TEST_CASE("functions", "[Lexer+Parser+Interpreter]")
{
    REQUIRE_THROWS_AS(interpret(L"func funkcja(int a, str b) {return true;}"), InvalidReturnError);
    REQUIRE_THROWS_AS(interpret(L"func funkcja(int a, str b) -> int {}"), InvalidReturnError);
    REQUIRE(
        interpret(L"func funkcja(int$ a)\n"
                  L"{\n"
                  L"    if(a === 3) {\n"
                  L"        return;\n"
                  L"    }\n"
                  L"    a = a + 1;\n"
                  L"}\n"
                  L"\n"
                  L"func main() {\n"
                  L"    int$ a = 2;\n"
                  L"    funkcja(a); print(a); # 3\n"
                  L"    funkcja(a); print(a); # 3\n"
                  L"}\n") == L"33"
    );
    REQUIRE_THROWS_AS(interpret(L"func f(int a, int$ b) {a = 3;}"), ImmutableError);
    REQUIRE(
        interpret(L"func f(int a, int$ b) {b = 4;}\n"
                  L"func main() {\n"
                  L"    int a = 1;\n"
                  L"    int b = 1;\n"
                  L"    f(a, b);\n"
                  L"    print(a ! b);\n"
                  L"}\n") == L"14"
    );
    REQUIRE(
        interpret(L"func factorial(int n) -> int {\n"
                  L"    if(n == 0 or n == 1) {\n"
                  L"        return 1;\n"
                  L"    }\n"
                  L"    else {\n"
                  L"        return n * factorial(n - 1);\n"
                  L"    }\n"
                  L"}\n"
                  L"\n"
                  L"func main() {\n"
                  L"    int a = factorial(3); println(a);\n"
                  L"    int b = factorial(4); println(b);\n"
                  L"}\n") == L"6\n24\n"
    );
    REQUIRE_THROWS_AS(
        interpret(L"func factorial(int n) -> int {\n"
                  L"    return n * factorial(n - 1);\n"
                  L"}\n"
                  L"\n"
                  L"func main() {\n"
                  L"    int a = factorial(3); # błąd - przekroczono limit wywołań funkcji\n"
                  L"}\n"),
        StackOverflowError
    );
    REQUIRE(
        interpret(L"func f(int a, str b) -> int { return 1; }\n"
                  L"func f(int a, int b) -> int { return 2; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    int c = f(1, 2); print(c);\n"
                  L"}\n") == L"2"
    );
    REQUIRE_THROWS_AS(
        interpret(L"func f(int a, str b) -> int { return 1; }\n"
                  L"func f(int a, int b) -> int { return 2; }\n"
                  L"\n"
                  L"func main() {\n"
                  L"    f(1, 2.5);\n"
                  L"}\n"),
        AmbiguousFunctionCallError
    );
    REQUIRE_THROWS_AS(
        interpret(L"func f(int a) {}\n"
                  L"func f(int$ a) {}\n"),
        DuplicateFunctionError
    );
    REQUIRE_THROWS_AS(
        interpret(L"func f(int a) {}\n"
                  L"func f(int a) -> int { return 2; }\n"),
        DuplicateFunctionError
    );
}

TEST_CASE("variable visibility", "[Lexer+Parser+Interpreter]")
{
    REQUIRE_THROWS_AS(
        interpret(wrapInMain(L"if(true) { int a = 4; }\n"
                             L"int b = a + 2;\n")),
        UnknownVariableError
    );
    REQUIRE_THROWS_AS(
        interpret(L"func f(int a) {\n"
                  L"    str a = \"a\";\n"
                  L"}\n"),
        VariableNameCollisionError
    );
    REQUIRE_THROWS_AS(
        interpret(wrapInMain(L"int a = 0;\n"
                             L"if(a == 0) {\n"
                             L"    int a = 1;\n"
                             L"}\n")),
        VariableNameCollisionError
    );
}
