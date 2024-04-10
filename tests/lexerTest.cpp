#include "lexer.hpp"

#include "lexerExceptions.hpp"
#include "streamReader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using enum TokenType;

Token lexSingleToken(std::wstring inputString)
{
    std::wstringstream input(inputString);
    StreamReader reader(input);
    Lexer lexer(reader);
    return lexer.getNextToken();
}

void checkToken(std::wstring inputString, TokenType tokenType)
{
    REQUIRE(lexSingleToken(inputString).type == tokenType);
}

void checkToken(std::wstring inputString, TokenType tokenType, std::wstring tokenValue)
{
    Token token = lexSingleToken(inputString);
    REQUIRE(token.type == tokenType);
    REQUIRE(std::get<std::wstring>(token.value) == tokenValue);
}

void checkToken(std::wstring inputString, TokenType tokenType, double tokenValue)
{
    Token token = lexSingleToken(inputString);
    REQUIRE(token.type == tokenType);
    REQUIRE_THAT(std::get<double>(token.value), Catch::Matchers::WithinAbsMatcher(tokenValue, 1e-6));
}

void checkToken(std::wstring inputString, TokenType tokenType, int32_t tokenValue)
{
    Token token = lexSingleToken(inputString);
    REQUIRE(token.type == tokenType);
    REQUIRE(std::get<int32_t>(token.value) == tokenValue);
}

template <typename ErrorType>
void checkTokenError(std::wstring inputString)
{
    std::wstringstream input(inputString);
    StreamReader reader(input);
    Lexer lexer(reader);
    try
    {
        lexer.getNextToken();
    }
    catch(const ErrorType &e)
    {
        REQUIRE(e.getPosition() == Position{1, 1});
    }
}

TEST_CASE("keywords tokens", "[Lexer]")
{
    checkToken(L"include", KW_INCLUDE);
    checkToken(L"struct", KW_STRUCT);
    checkToken(L"variant", KW_VARIANT);
    checkToken(L"func", KW_FUNC);
    checkToken(L"continue", KW_CONTINUE);
    checkToken(L"break", KW_BREAK);
    checkToken(L"return", KW_RETURN);
    checkToken(L"if", KW_IF);
    checkToken(L"elif", KW_ELIF);
    checkToken(L"else", KW_ELSE);
    checkToken(L"while", KW_WHILE);
    checkToken(L"do", KW_DO);
    checkToken(L"is", KW_IS);
    checkToken(L"or", KW_OR);
    checkToken(L"xor", KW_XOR);
    checkToken(L"and", KW_AND);
    checkToken(L"not", KW_NOT);
    checkToken(L"int", KW_INT);
    checkToken(L"float", KW_FLOAT);
    checkToken(L"bool", KW_BOOL);
    checkToken(L"str", KW_STR);
}

TEST_CASE("bool literal tokens", "[Lexer]")
{
    checkToken(L"true", TRUE_LITERAL);
    checkToken(L"false", FALSE_LITERAL);
}

TEST_CASE("operator tokens", "[Lexer]")
{
    checkToken(L"=", OP_ASSIGN);
    checkToken(L".", OP_DOT);
    checkToken(L"==", OP_EQUAL);
    checkToken(L"!=", OP_NOT_EQUAL);
    checkToken(L"===", OP_IDENTICAL);
    checkToken(L"!==", OP_NOT_IDENTICAL);
    checkToken(L"!", OP_CONCAT);
    checkToken(L"@", OP_STR_MULTIPLY);
    checkToken(L">", OP_GREATER);
    checkToken(L"<", OP_LESSER);
    checkToken(L">=", OP_GREATER_EQUAL);
    checkToken(L"<=", OP_LESSER_EQUAL);
    checkToken(L"+", OP_PLUS);
    checkToken(L"-", OP_MINUS);
    checkToken(L"*", OP_MULTIPLY);
    checkToken(L"/", OP_DIVIDE);
    checkToken(L"//", OP_FLOOR_DIVIDE);
    checkToken(L"%", OP_MODULO);
    checkToken(L"**", OP_EXPONENT);
}

TEST_CASE("control sequence tokens", "[Lexer]")
{
    checkToken(L"{", LBRACE);
    checkToken(L"}", RBRACE);
    checkToken(L";", SEMICOLON);
    checkToken(L"(", LPAREN);
    checkToken(L")", RPAREN);
    checkToken(L"->", ARROW);
    checkToken(L",", COMMA);
    checkToken(L"$", DOLLAR_SIGN);
    checkToken(L"[", LSQUAREBRACE);
    checkToken(L"]", RSQUAREBRACE);
    checkToken(L"", EOT);
}

TEST_CASE("comments", "[Lexer]")
{
    checkToken(L"# this is a comment", COMMENT, L" this is a comment");
    checkToken(L"# double # hash #comment", COMMENT, L" double # hash #comment");
    checkToken(L"  \n # this is a comment\n hehe xd", COMMENT, L" this is a comment");
    std::wstring longString(Lexer::MAX_COMMENT_SIZE, L'a');
    checkToken(L"#" + longString, COMMENT, longString);
    checkTokenError<CommentTooLongError>(L"#A" + longString);
}

TEST_CASE("strings", "[Lexer]")
{
    checkToken(L"\"\"", STR_LITERAL, L"");
    checkToken(L"\"abcd XD\"", STR_LITERAL, L"abcd XD");
    checkToken(L"\"część\"", STR_LITERAL, L"część");
    checkToken(L"\"czę\\tść\"", STR_LITERAL, L"czę\tść");
    checkToken(L"\"czę\\rść\"", STR_LITERAL, L"czę\rść");
    checkToken(L"\"czę\\nść\"", STR_LITERAL, L"czę\nść");
    checkToken(L"\"czę\\\"ść\"", STR_LITERAL, L"czę\"ść");
    checkToken(L"\"czę\\\\ść\"", STR_LITERAL, L"czę\\ść");
    checkToken(L"\"czę\\x13\\x20\\xbF\"", STR_LITERAL, L"czę\x13 \xBF");
    std::wstring longString(Lexer::MAX_STRING_SIZE, L'A');
    checkToken(L"\"" + longString + L"\"", STR_LITERAL, longString);
    checkTokenError<InvalidHexCharError>(L"\"\\xag\"");
    checkTokenError<UnknownEscapeSequenceError>(L"\"\\k\"");
    checkTokenError<NewlineInStringError>(L"\"abc\ndef\"");
    checkTokenError<UnterminatedStringError>(L"\"abcdef");
    checkTokenError<StringTooLongError>(L"\"A" + longString + L"\"");
    checkTokenError<UnterminatedStringError>(L"\"abc\\");
    checkTokenError<NewlineInStringError>(L"\"abc\\\n");
}

TEST_CASE("integer literals", "[Lexer]")
{
    checkToken(L"234", INT_LITERAL, 234);
    checkToken(L"0", INT_LITERAL, 0);
    checkToken(L"-345", OP_MINUS);
    checkTokenError<IntWithLeadingZeroError>(L"0123");
    checkTokenError<IntTooLargeError>(std::format(L"{}", static_cast<int64_t>(INT32_MAX) + 1));
}

TEST_CASE("float literals", "[Lexer]")
{
    checkToken(L"1.001", FLOAT_LITERAL, 1.001);
    checkToken(L"234.432", FLOAT_LITERAL, 234.432);
    checkToken(L"-234.432", OP_MINUS);
    checkToken(L"234.", FLOAT_LITERAL, 234.);
    checkToken(L"234.00", FLOAT_LITERAL, 234.);
    checkToken(L"0.0", FLOAT_LITERAL, 0.0);
    checkToken(L"2.2e20", FLOAT_LITERAL, 2.2e20);
    checkToken(L"2.2E-20", FLOAT_LITERAL, 2.2E-20);
    checkToken(L"2.2E0", FLOAT_LITERAL, 2.2);
    checkToken(L"2.2E-0", FLOAT_LITERAL, 2.2);
    checkToken(L"2.2E01", FLOAT_LITERAL, 22.0);
    checkToken(L"2.2E-01", FLOAT_LITERAL, 0.22);
    checkToken(L"2E01", FLOAT_LITERAL, 20.0);
    checkToken(L"2E-01", FLOAT_LITERAL, 0.2);
    int64_t largeInt = static_cast<int64_t>(INT32_MAX);
    checkToken(std::format(L"{}.0", largeInt), FLOAT_LITERAL, static_cast<double>(largeInt));
    checkToken(std::format(L"{}.E2", largeInt), FLOAT_LITERAL, static_cast<double>(largeInt) * 100);
    int64_t tooLargeInt = static_cast<int64_t>(INT32_MAX) + 1;
    checkTokenError<IntTooLargeError>(std::format(L"{}.2", tooLargeInt));
    checkTokenError<IntTooLargeError>(std::format(L"2.{}", tooLargeInt));
    checkTokenError<IntTooLargeError>(std::format(L"1e-{}", tooLargeInt));
    checkTokenError<InvalidExponentError>(std::format(L"1e-", tooLargeInt));
    checkTokenError<IntWithLeadingZeroError>(L"012.3");
}

TEST_CASE("identifiers", "[Lexer]")
{
    checkToken(L"ifident", IDENTIFIER, L"ifident");
    checkToken(L"identifier", IDENTIFIER, L"identifier");
    checkToken(L"_1dඞ'nt_1fi3文'", IDENTIFIER, L"_1dඞ'nt_1fi3文'");
    checkToken(L"2ident", INT_LITERAL, 2);
    std::wstring longString(Lexer::MAX_IDENTIFIER_SIZE, L'I');
    checkToken(longString, IDENTIFIER, longString);
    checkTokenError<IdentifierTooLongError>(L"A" + longString);
}

TEST_CASE("unknown token", "[Lexer]")
{
    checkTokenError<UnknownTokenError>(L"^");
    checkTokenError<UnknownTokenError>(L"\\n");
}

void getAndCheckToken(ILexer &lexer, TokenType tokenType)
{
    REQUIRE(lexer.getNextToken().type == tokenType);
}

void getAndCheckToken(ILexer &lexer, TokenType tokenType, std::wstring tokenValue)
{
    Token token = lexer.getNextToken();
    REQUIRE(token.type == tokenType);
    REQUIRE(std::get<std::wstring>(token.value) == tokenValue);
}

void getAndCheckToken(ILexer &lexer, TokenType tokenType, int32_t tokenValue)
{
    Token token = lexer.getNextToken();
    REQUIRE(token.type == tokenType);
    REQUIRE(std::get<int32_t>(token.value) == tokenValue);
}

TEST_CASE("no whitespace input", "[Lexer]")
{
    std::wstringstream input(L"abc+-abc");
    StreamReader reader(input);
    Lexer lexer(reader);

    getAndCheckToken(lexer, IDENTIFIER, L"abc");
    getAndCheckToken(lexer, OP_PLUS);
    getAndCheckToken(lexer, OP_MINUS);
    getAndCheckToken(lexer, IDENTIFIER, L"abc");
}

TEST_CASE("a lot of whitespace input", "[Lexer]")
{
    std::wstringstream input(L"abc\t  +  \t-\n\n\nabc");
    StreamReader reader(input);
    Lexer lexer(reader);

    getAndCheckToken(lexer, IDENTIFIER, L"abc");
    getAndCheckToken(lexer, OP_PLUS);
    getAndCheckToken(lexer, OP_MINUS);
    getAndCheckToken(lexer, IDENTIFIER, L"abc");
}

TEST_CASE("factorial code example", "[Lexer]")
{
    std::wstringstream input(L"func factorial(int n) -> int {\n"
                             "    if(n == 0 or n == 1) {\n"
                             "        return 1;\n"
                             "    }\n"
                             "    else {\n"
                             "        return n * factorial(n - 1);\n"
                             "    }\n"
                             "}\n"
                             "\n");
    StreamReader reader(input);
    Lexer lexer(reader);

    getAndCheckToken(lexer, KW_FUNC);
    getAndCheckToken(lexer, IDENTIFIER, L"factorial");
    getAndCheckToken(lexer, LPAREN);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, IDENTIFIER, L"n");
    getAndCheckToken(lexer, RPAREN);
    getAndCheckToken(lexer, ARROW);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, KW_IF);
    getAndCheckToken(lexer, LPAREN);
    getAndCheckToken(lexer, IDENTIFIER, L"n");
    getAndCheckToken(lexer, OP_EQUAL);
    getAndCheckToken(lexer, INT_LITERAL, 0);
    getAndCheckToken(lexer, KW_OR);
    getAndCheckToken(lexer, IDENTIFIER, L"n");
    getAndCheckToken(lexer, OP_EQUAL);
    getAndCheckToken(lexer, INT_LITERAL, 1);
    getAndCheckToken(lexer, RPAREN);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, KW_RETURN);
    getAndCheckToken(lexer, INT_LITERAL, 1);
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, KW_ELSE);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, KW_RETURN);
    getAndCheckToken(lexer, IDENTIFIER, L"n");
    getAndCheckToken(lexer, OP_MULTIPLY);
    getAndCheckToken(lexer, IDENTIFIER, L"factorial");
    getAndCheckToken(lexer, LPAREN);
    getAndCheckToken(lexer, IDENTIFIER, L"n");
    getAndCheckToken(lexer, OP_MINUS);
    getAndCheckToken(lexer, INT_LITERAL, 1);
    getAndCheckToken(lexer, RPAREN);
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, EOT);
    getAndCheckToken(lexer, EOT);
    getAndCheckToken(lexer, EOT);
}

TEST_CASE("nested structs example", "[Lexer]")
{
    std::wstringstream input(L"struct S1 {\n"
                             "    int a;\n"
                             "    int b;\n"
                             "}\n"
                             "variant V {\n"
                             "    S1 s;\n"
                             "    int i;\n"
                             "}\n"
                             "struct S2 {\n"
                             "    V v;\n"
                             "    int i;\n"
                             "}\n"
                             "\n"
                             "func main() {\n"
                             "    S2 s = {S1({2, 3}), 2};\n"
                             "    # konieczne jest podanie typu S1\n"
                             "}\n");
    StreamReader reader(input);
    Lexer lexer(reader);

    getAndCheckToken(lexer, KW_STRUCT);
    getAndCheckToken(lexer, IDENTIFIER, L"S1");
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, IDENTIFIER, L"a");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, IDENTIFIER, L"b");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, KW_VARIANT);
    getAndCheckToken(lexer, IDENTIFIER, L"V");
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, IDENTIFIER, L"S1");
    getAndCheckToken(lexer, IDENTIFIER, L"s");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, IDENTIFIER, L"i");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, KW_STRUCT);
    getAndCheckToken(lexer, IDENTIFIER, L"S2");
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, IDENTIFIER, L"V");
    getAndCheckToken(lexer, IDENTIFIER, L"v");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, KW_INT);
    getAndCheckToken(lexer, IDENTIFIER, L"i");
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, KW_FUNC);
    getAndCheckToken(lexer, IDENTIFIER, L"main");
    getAndCheckToken(lexer, LPAREN);
    getAndCheckToken(lexer, RPAREN);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, IDENTIFIER, L"S2");
    getAndCheckToken(lexer, IDENTIFIER, L"s");
    getAndCheckToken(lexer, OP_ASSIGN);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, IDENTIFIER, L"S1");
    getAndCheckToken(lexer, LPAREN);
    getAndCheckToken(lexer, LBRACE);
    getAndCheckToken(lexer, INT_LITERAL, 2);
    getAndCheckToken(lexer, COMMA);
    getAndCheckToken(lexer, INT_LITERAL, 3);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, RPAREN);
    getAndCheckToken(lexer, COMMA);
    getAndCheckToken(lexer, INT_LITERAL, 2);
    getAndCheckToken(lexer, RBRACE);
    getAndCheckToken(lexer, SEMICOLON);
    getAndCheckToken(lexer, COMMENT, L" konieczne jest podanie typu S1");
    getAndCheckToken(lexer, RBRACE);
}
