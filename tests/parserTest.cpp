#include "parser.hpp"

#include "fakeLexer.hpp"
#include "parserExceptions.hpp"
#include "printingVisitor.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include <sstream>

using enum TokenType;

template <typename TokenContainer>
void checkParsing(TokenContainer tokens, std::wstring expected)
{
    FakeLexer lexer(tokens);
    Parser parser(lexer);
    Program documentTree = parser.parseProgram();
    std::wstringstream output;
    PrintingVisitor printer(output);
    documentTree.accept(printer);
    REQUIRE(output.str() == expected);
}

template <typename ErrorType, typename TokenContainer>
void checkParseError(TokenContainer tokens)
{
    FakeLexer lexer(tokens);
    Parser parser(lexer);
    REQUIRE_THROWS_AS(parser.parseProgram(), ErrorType);
}

TEST_CASE("empty Program", "[Parser]")
{
    std::array tokens = {
        Token(EOT, {1, 1}),
    };
    checkParsing(tokens, L"Program containing:\n");
}

TEST_CASE("IncludeStatement", "[Parser]")
{
    std::vector tokens = {
        Token(KW_INCLUDE, {1, 1}),
        Token(STR_LITERAL, {1, 3}, L"file.txt"),
        Token(SEMICOLON, {1, 6}),
        Token(EOT, {1, 9}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Includes:\n"
                L"`-IncludeStatement <line: 1, col: 1> filePath=file.txt\n"
    );
}

TEST_CASE("IncludeStatement errors", "[Parser]")
{
    std::vector tokens = {
        Token(KW_INCLUDE, {1, 1}),
        Token(SEMICOLON, {1, 6}),
        Token(EOT, {1, 9}),
    };
    checkParseError<SyntaxError>(tokens); // no file name
    tokens = {
        Token(KW_INCLUDE, {1, 1}),
        Token(STR_LITERAL, {1, 3}, L"file.txt"),
        Token(EOT, {1, 9}),
    };
    checkParseError<SyntaxError>(tokens); // no semicolon
}

TEST_CASE("StructDeclaration", "[Parser]")
{
    std::array tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Structs:\n"
                L"`-a_structure: StructDeclaration <line: 1, col: 1>\n"
                L" |-Field <line: 1, col: 9> type=int name=field_name\n"
                L" `-Field <line: 2, col: 1> type=typename name=field_name_2\n"
    );
}

TEST_CASE("StructDeclaration errors", "[Parser]")
{
    std::vector tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no struct name
    tokens = {
        Token(KW_STRUCT, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),    Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // zero struct fields
    tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // left brace missing
    tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // right brace missing
    tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),
        Token(KW_IF, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // invalid token instead of field type
    tokens = {
        Token(KW_STRUCT, {1, 1}),  Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),     Token(KW_INT, {1, 9}),
        Token(SEMICOLON, {1, 15}), Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no field name
    tokens = {
        Token(KW_STRUCT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_structure"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no semicolon after field declaration
}

TEST_CASE("VariantDeclaration", "[Parser]")
{
    std::array tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Variants:\n"
                L"`-a_variant: VariantDeclaration <line: 1, col: 1>\n"
                L" |-Field <line: 1, col: 9> type=int name=field_name\n"
                L" `-Field <line: 2, col: 1> type=typename name=field_name_2\n"
    );
}

TEST_CASE("VariantDeclaration errors", "[Parser]")
{
    std::vector tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no variant name
    tokens = {
        Token(KW_VARIANT, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),     Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // zero variant fields
    tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // left brace missing
    tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(SEMICOLON, {2, 8}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // right brace missing
    tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),
        Token(KW_IF, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // invalid token instead of field type
    tokens = {
        Token(KW_VARIANT, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),     Token(KW_INT, {1, 9}),
        Token(SEMICOLON, {1, 15}), Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no field name
    tokens = {
        Token(KW_VARIANT, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_variant"),
        Token(LBRACE, {1, 6}),
        Token(KW_INT, {1, 9}),
        Token(IDENTIFIER, {1, 12}, L"field_name"),
        Token(SEMICOLON, {1, 15}),
        Token(IDENTIFIER, {2, 1}, L"typename"),
        Token(IDENTIFIER, {2, 3}, L"field_name_2"),
        Token(RBRACE, {3, 1}),
        Token(EOT, {3, 2}),
    };
    checkParseError<SyntaxError>(tokens); // no semicolon after field declaration
}

TEST_CASE("FunctionDeclaration - empty function", "[Parser]")
{
    std::array tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),  Token(RPAREN, {1, 9}),
        Token(LBRACE, {1, 12}), Token(RBRACE, {1, 15}),
        Token(EOT, {1, 18}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
    );
}

TEST_CASE("FunctionDeclaration - parameter list and return type", "[Parser]")
{
    std::array tokens = {
        Token(KW_FUNC, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),
        Token(KW_STR, {1, 9}),
        Token(DOLLAR_SIGN, {1, 14}),
        Token(IDENTIFIER, {1, 15}, L"param1"),
        Token(COMMA, {1, 26}),
        Token(IDENTIFIER, {1, 27}, L"typename"),
        Token(IDENTIFIER, {2, 1}, L"param2"),
        Token(RPAREN, {2, 9}),
        Token(ARROW, {2, 13}),
        Token(IDENTIFIER, {2, 15}, L"type_name"),
        Token(LBRACE, {3, 12}),
        Token(RBRACE, {3, 15}),
        Token(EOT, {3, 18}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1> returnType=type_name\n"
                L" Parameters:\n"
                L" |-VariableDeclaration <line: 1, col: 9> type=str name=param1 mutable=1\n"
                L" `-VariableDeclaration <line: 1, col: 27> type=typename name=param2 mutable=0\n"
    );
}

TEST_CASE("FunctionDeclaration - errors", "[Parser]")
{
    std::vector tokens = {
        Token(KW_FUNC, {1, 1}), Token(LPAREN, {1, 6}),  Token(RPAREN, {2, 9}),
        Token(LBRACE, {3, 12}), Token(RBRACE, {3, 15}), Token(EOT, {3, 18}),
    };
    checkParseError<SyntaxError>(tokens); // no function name
    tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LBRACE, {1, 12}), Token(RBRACE, {1, 15}),
        Token(EOT, {1, 18}),
    };
    checkParseError<SyntaxError>(tokens); // parameter parentheses missing
    tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),  Token(LBRACE, {1, 12}),
        Token(RBRACE, {1, 15}), Token(EOT, {1, 18}),
    };
    checkParseError<SyntaxError>(tokens); // parameter list not closed
    tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"), Token(LPAREN, {1, 6}), Token(RPAREN, {1, 9}),
        Token(EOT, {1, 18}),
    };
    checkParseError<SyntaxError>(tokens); // missing body
    tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),  Token(RPAREN, {1, 9}),
        Token(LBRACE, {1, 12}), Token(EOT, {1, 18}),
    };
    checkParseError<SyntaxError>(tokens); // body not closed
    tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),  Token(RPAREN, {1, 9}),
        Token(ARROW, {2, 1}),   Token(LBRACE, {1, 12}),
        Token(RBRACE, {1, 15}), Token(EOT, {1, 18}),
    };
    checkParseError<SyntaxError>(tokens); // invalid return type
}

TEST_CASE("FunctionDeclaration parameters - errors", "[Parser]")
{
    std::vector tokens = {
        Token(KW_FUNC, {1, 1}), Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),  Token(KW_IF, {1, 9}),
        Token(RPAREN, {2, 9}),  Token(LBRACE, {3, 12}),
        Token(RBRACE, {3, 15}), Token(EOT, {3, 18}),
    };
    checkParseError<SyntaxError>(tokens); // invalid token in place of parameter
    tokens = {
        Token(KW_FUNC, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),
        Token(KW_STR, {1, 9}),
        Token(DOLLAR_SIGN, {1, 14}),
        Token(COMMA, {1, 26}),
        Token(IDENTIFIER, {1, 27}, L"typename"),
        Token(IDENTIFIER, {2, 1}, L"param2"),
        Token(RPAREN, {2, 9}),
        Token(ARROW, {2, 13}),
        Token(IDENTIFIER, {2, 15}, L"type_name"),
        Token(LBRACE, {3, 12}),
        Token(RBRACE, {3, 15}),
        Token(EOT, {3, 18}),
    };
    checkParseError<SyntaxError>(tokens); // parameter name missing
    tokens = {
        Token(KW_FUNC, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),
        Token(IDENTIFIER, {1, 27}, L"typename"),
        Token(IDENTIFIER, {2, 1}, L"param2"),
        Token(COMMA, {2, 8}),
        Token(RPAREN, {2, 9}),
        Token(ARROW, {2, 13}),
        Token(IDENTIFIER, {2, 15}, L"type_name"),
        Token(LBRACE, {3, 12}),
        Token(RBRACE, {3, 15}),
        Token(EOT, {3, 18}),
    };
    checkParseError<SyntaxError>(tokens); // trailing comma in parameter list
}
