#include "lexer.hpp"

#include "streamReader.hpp"
#include "testErrorHandler.hpp"

#include <catch2/catch_test_macros.hpp>

Token lexSingleToken(std::wstring inputString)
{
    TestErrorHandler errorHandler;
    std::wstringstream input(inputString);
    StreamReader reader(input, errorHandler);
    Lexer lexer(reader, errorHandler);
    return lexer.getNextToken();
}

TEST_CASE("include token", "[Lexer]")
{
    Token token = lexSingleToken(L"include");
    REQUIRE(token.type == TokenType::KW_INCLUDE);
}

TEST_CASE("struct token", "[Lexer]")
{
    Token token = lexSingleToken(L"struct");
    REQUIRE(token.type == TokenType::KW_STRUCT);
}

TEST_CASE("variant token", "[Lexer]")
{
    Token token = lexSingleToken(L"variant");
    REQUIRE(token.type == TokenType::KW_VARIANT);
}

TEST_CASE("func token", "[Lexer]")
{
    Token token = lexSingleToken(L"func");
    REQUIRE(token.type == TokenType::KW_FUNC);
}

TEST_CASE("continue token", "[Lexer]")
{
    Token token = lexSingleToken(L"continue");
    REQUIRE(token.type == TokenType::KW_CONTINUE);
}

TEST_CASE("break token", "[Lexer]")
{
    Token token = lexSingleToken(L"break");
    REQUIRE(token.type == TokenType::KW_BREAK);
}

TEST_CASE("return token", "[Lexer]")
{
    Token token = lexSingleToken(L"return");
    REQUIRE(token.type == TokenType::KW_RETURN);
}

TEST_CASE("if token", "[Lexer]")
{
    Token token = lexSingleToken(L"if");
    REQUIRE(token.type == TokenType::KW_IF);
}

TEST_CASE("elif token", "[Lexer]")
{
    Token token = lexSingleToken(L"elif");
    REQUIRE(token.type == TokenType::KW_ELIF);
}

TEST_CASE("else token", "[Lexer]")
{
    Token token = lexSingleToken(L"else");
    REQUIRE(token.type == TokenType::KW_ELSE);
}

TEST_CASE("while token", "[Lexer]")
{
    Token token = lexSingleToken(L"while");
    REQUIRE(token.type == TokenType::KW_WHILE);
}

TEST_CASE("do token", "[Lexer]")
{
    Token token = lexSingleToken(L"do");
    REQUIRE(token.type == TokenType::KW_DO);
}

TEST_CASE("is token", "[Lexer]")
{
    Token token = lexSingleToken(L"is");
    REQUIRE(token.type == TokenType::KW_IS);
}

TEST_CASE("or token", "[Lexer]")
{
    Token token = lexSingleToken(L"or");
    REQUIRE(token.type == TokenType::KW_OR);
}

TEST_CASE("xor token", "[Lexer]")
{
    Token token = lexSingleToken(L"xor");
    REQUIRE(token.type == TokenType::KW_XOR);
}

TEST_CASE("and token", "[Lexer]")
{
    Token token = lexSingleToken(L"and");
    REQUIRE(token.type == TokenType::KW_AND);
}

TEST_CASE("not token", "[Lexer]")
{
    Token token = lexSingleToken(L"not");
    REQUIRE(token.type == TokenType::KW_NOT);
}

TEST_CASE("int token", "[Lexer]")
{
    Token token = lexSingleToken(L"int");
    REQUIRE(token.type == TokenType::KW_INT);
}

TEST_CASE("float token", "[Lexer]")
{
    Token token = lexSingleToken(L"float");
    REQUIRE(token.type == TokenType::KW_FLOAT);
}

TEST_CASE("bool token", "[Lexer]")
{
    Token token = lexSingleToken(L"bool");
    REQUIRE(token.type == TokenType::KW_BOOL);
}

TEST_CASE("str token", "[Lexer]")
{
    Token token = lexSingleToken(L"str");
    REQUIRE(token.type == TokenType::KW_STR);
}

TEST_CASE("{ token", "[Lexer]")
{
    Token token = lexSingleToken(L"{");
    REQUIRE(token.type == TokenType::LBRACE);
}

TEST_CASE("} token", "[Lexer]")
{
    Token token = lexSingleToken(L"}");
    REQUIRE(token.type == TokenType::RBRACE);
}

TEST_CASE("; token", "[Lexer]")
{
    Token token = lexSingleToken(L";");
    REQUIRE(token.type == TokenType::SEMICOLON);
}

TEST_CASE("( token", "[Lexer]")
{
    Token token = lexSingleToken(L"(");
    REQUIRE(token.type == TokenType::LPAREN);
}

TEST_CASE(") token", "[Lexer]")
{
    Token token = lexSingleToken(L")");
    REQUIRE(token.type == TokenType::RPAREN);
}

TEST_CASE("-> token", "[Lexer]")
{
    Token token = lexSingleToken(L"->");
    REQUIRE(token.type == TokenType::ARROW);
}

TEST_CASE(", token", "[Lexer]")
{
    Token token = lexSingleToken(L",");
    REQUIRE(token.type == TokenType::COMMA);
}

TEST_CASE("$ token", "[Lexer]")
{
    Token token = lexSingleToken(L"$");
    REQUIRE(token.type == TokenType::DOLLAR_SIGN);
}

TEST_CASE("= token", "[Lexer]")
{
    Token token = lexSingleToken(L"=");
    REQUIRE(token.type == TokenType::OP_ASSIGN);
}

TEST_CASE(". token", "[Lexer]")
{
    Token token = lexSingleToken(L".");
    REQUIRE(token.type == TokenType::OP_DOT);
}

TEST_CASE("== token", "[Lexer]")
{
    Token token = lexSingleToken(L"==");
    REQUIRE(token.type == TokenType::OP_EQUAL);
}

TEST_CASE("!= token", "[Lexer]")
{
    Token token = lexSingleToken(L"!=");
    REQUIRE(token.type == TokenType::OP_NOT_EQUAL);
}

TEST_CASE("=== token", "[Lexer]")
{
    Token token = lexSingleToken(L"===");
    REQUIRE(token.type == TokenType::OP_IDENTICAL);
}

TEST_CASE("!== token", "[Lexer]")
{
    Token token = lexSingleToken(L"!==");
    REQUIRE(token.type == TokenType::OP_NOT_IDENTICAL);
}

TEST_CASE("! token", "[Lexer]")
{
    Token token = lexSingleToken(L"!");
    REQUIRE(token.type == TokenType::OP_CONCAT);
}

TEST_CASE("@ token", "[Lexer]")
{
    Token token = lexSingleToken(L"@");
    REQUIRE(token.type == TokenType::OP_STR_MULTIPLY);
}

TEST_CASE("> token", "[Lexer]")
{
    Token token = lexSingleToken(L">");
    REQUIRE(token.type == TokenType::OP_GREATER);
}

TEST_CASE("< token", "[Lexer]")
{
    Token token = lexSingleToken(L"<");
    REQUIRE(token.type == TokenType::OP_LESSER);
}

TEST_CASE(">= token", "[Lexer]")
{
    Token token = lexSingleToken(L">=");
    REQUIRE(token.type == TokenType::OP_GREATER_EQUAL);
}

TEST_CASE("<= token", "[Lexer]")
{
    Token token = lexSingleToken(L"<=");
    REQUIRE(token.type == TokenType::OP_LESSER_EQUAL);
}

TEST_CASE("+ token", "[Lexer]")
{
    Token token = lexSingleToken(L"+");
    REQUIRE(token.type == TokenType::OP_PLUS);
}

TEST_CASE("- token", "[Lexer]")
{
    Token token = lexSingleToken(L"-");
    REQUIRE(token.type == TokenType::OP_MINUS);
}

TEST_CASE("* token", "[Lexer]")
{
    Token token = lexSingleToken(L"*");
    REQUIRE(token.type == TokenType::OP_MULTIPLY);
}

TEST_CASE("/ token", "[Lexer]")
{
    Token token = lexSingleToken(L"/");
    REQUIRE(token.type == TokenType::OP_DIVIDE);
}

TEST_CASE("// token", "[Lexer]")
{
    Token token = lexSingleToken(L"//");
    REQUIRE(token.type == TokenType::OP_FLOOR_DIVIDE);
}

TEST_CASE("% token", "[Lexer]")
{
    Token token = lexSingleToken(L"%");
    REQUIRE(token.type == TokenType::OP_MODULO);
}

TEST_CASE("** token", "[Lexer]")
{
    Token token = lexSingleToken(L"**");
    REQUIRE(token.type == TokenType::OP_EXPONENT);
}

TEST_CASE("[ token", "[Lexer]")
{
    Token token = lexSingleToken(L"[");
    REQUIRE(token.type == TokenType::LSQUAREBRACE);
}

TEST_CASE("] token", "[Lexer]")
{
    Token token = lexSingleToken(L"]");
    REQUIRE(token.type == TokenType::RSQUAREBRACE);
}
