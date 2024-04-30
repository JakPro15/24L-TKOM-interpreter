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

std::vector<Token> wrapInFunction(std::vector<Token> tokens)
{
    std::vector wrapped = {
        Token(KW_FUNC, {1, 1}),
        Token(IDENTIFIER, {1, 3}, L"a_function"),
        Token(LPAREN, {1, 6}),
        Token(RPAREN, {2, 9}),
        Token(LBRACE, {3, 12}),
        // here will be the body of the function
        Token(RBRACE, {6, 1}),
        Token(EOT, {6, 2}),
    };
    wrapped.insert(wrapped.begin() + 5, tokens.begin(), tokens.end());
    return wrapped;
}

TEST_CASE("empty Program", "[Parser]")
{
    std::vector tokens = {
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
    std::vector tokens = {
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
    std::vector tokens = {
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
    checkParsing(
        wrapInFunction({}), L"Program containing:\n"
                            L"Functions:\n"
                            L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
    );
}

TEST_CASE("FunctionDeclaration - parameter list and return type", "[Parser]")
{
    std::vector tokens = {
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
                L" `-Parameters:\n"
                L"  |-VariableDeclaration <line: 1, col: 9> type=str name=param1 mutable=1\n"
                L"  `-VariableDeclaration <line: 1, col: 27> type=typename name=param2 mutable=0\n"
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

TEST_CASE("VariableDeclStatement", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_INT, {4, 1}),
        Token(IDENTIFIER, {4, 5}, L"const_var"),
        Token(OP_ASSIGN, {4, 20}),
        Token(INT_LITERAL, {4, 22}, 2),
        Token(SEMICOLON, {4, 25}),
        Token(IDENTIFIER, {5, 1}, L"some_type"),
        Token(DOLLAR_SIGN, {5, 10}),
        Token(IDENTIFIER, {5, 11}, L"mut_var"),
        Token(OP_ASSIGN, {5, 20}),
        Token(INT_LITERAL, {5, 22}, 3),
        Token(SEMICOLON, {5, 23}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  |-VariableDeclStatement <line: 4, col: 1>\n"
                L"  ||-VariableDeclaration <line: 4, col: 1> type=int name=const_var mutable=0\n"
                L"  |`-Literal <line: 4, col: 22> value=2\n"
                L"  `-VariableDeclStatement <line: 5, col: 1>\n"
                L"   |-VariableDeclaration <line: 5, col: 1> type=some_type name=mut_var mutable=1\n"
                L"   `-Literal <line: 5, col: 22> value=3\n"
    );
}

TEST_CASE("VariableDeclStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_INT, {4, 1}),
        Token(DOLLAR_SIGN, {4, 10}),
        Token(OP_ASSIGN, {4, 20}),
        Token(INT_LITERAL, {4, 22}, 2),
        Token(SEMICOLON, {4, 25}),
    });
    checkParseError<SyntaxError>(tokens); // missing variable name - builtin type case
    tokens = wrapInFunction({
        Token(IDENTIFIER, {4, 1}, L"strct"),
        Token(DOLLAR_SIGN, {4, 10}),
        Token(OP_ASSIGN, {4, 20}),
        Token(INT_LITERAL, {4, 22}, 2),
        Token(SEMICOLON, {4, 25}),
    });
    checkParseError<SyntaxError>(tokens); // missing variable name - identifier type case
    tokens = wrapInFunction({
        Token(KW_INT, {4, 1}),
        Token(IDENTIFIER, {4, 5}, L"const_var"),
        Token(INT_LITERAL, {4, 22}, 2),
        Token(SEMICOLON, {4, 25}),
    });
    checkParseError<SyntaxError>(tokens); // missing =
    tokens = wrapInFunction({
        Token(KW_INT, {4, 1}),
        Token(IDENTIFIER, {4, 5}, L"const_var"),
        Token(OP_ASSIGN, {4, 20}),
        Token(SEMICOLON, {4, 25}),
    });
    checkParseError<SyntaxError>(tokens); // missing expression
    tokens = wrapInFunction({
        Token(KW_INT, {4, 1}),
        Token(IDENTIFIER, {4, 5}, L"const_var"),
        Token(OP_ASSIGN, {4, 20}),
        Token(INT_LITERAL, {4, 22}, 2),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon
}

TEST_CASE("AssignmentStatement", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(IDENTIFIER, {3, 5}, L"some_var"),
        Token(OP_ASSIGN, {3, 12}),
        Token(INT_LITERAL, {3, 14}, 2),
        Token(SEMICOLON, {3, 15}),
        Token(IDENTIFIER, {4, 1}, L"some_var"),
        Token(OP_DOT, {4, 8}),
        Token(IDENTIFIER, {4, 9}, L"some_var2"),
        Token(OP_DOT, {4, 18}),
        Token(IDENTIFIER, {4, 19}, L"some_var3"),
        Token(OP_ASSIGN, {4, 29}),
        Token(INT_LITERAL, {4, 31}, 3),
        Token(SEMICOLON, {4, 32}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  |-AssignmentStatement <line: 3, col: 5>\n"
                L"  ||-Assignable <line: 3, col: 5> right=some_var\n"
                L"  |`-Literal <line: 3, col: 14> value=2\n"
                L"  `-AssignmentStatement <line: 4, col: 1>\n"
                L"   |-Assignable <line: 4, col: 1> right=some_var3\n"
                L"   |`-Assignable <line: 4, col: 1> right=some_var2\n"
                L"   | `-Assignable <line: 4, col: 1> right=some_var\n"
                L"   `-Literal <line: 4, col: 31> value=3\n"
    );
}

TEST_CASE("AssignmentStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(IDENTIFIER, {4, 1}, L"some_var"),
        Token(INT_LITERAL, {4, 31}, 3),
        Token(SEMICOLON, {4, 32}),
    });
    checkParseError<SyntaxError>(tokens); // missing =
    tokens = wrapInFunction({
        Token(IDENTIFIER, {4, 1}, L"some_var"),
        Token(OP_DOT, {4, 8}),
        Token(OP_ASSIGN, {4, 29}),
        Token(INT_LITERAL, {4, 31}, 3),
        Token(SEMICOLON, {4, 32}),
    });
    checkParseError<SyntaxError>(tokens); // missing identifier after dot
    tokens = wrapInFunction({
        Token(IDENTIFIER, {4, 1}, L"some_var"),
        Token(OP_DOT, {4, 8}),
        Token(IDENTIFIER, {4, 9}, L"some_var2"),
        Token(INT_LITERAL, {4, 31}, 3),
        Token(SEMICOLON, {4, 32}),
    });
    checkParseError<SyntaxError>(tokens); // missing =
    tokens = wrapInFunction({
        Token(IDENTIFIER, {4, 1}, L"some_var"),
        Token(OP_ASSIGN, {4, 29}),
        Token(SEMICOLON, {4, 32}),
    });
    checkParseError<SyntaxError>(tokens); // missing expression
    tokens = wrapInFunction({
        Token(IDENTIFIER, {3, 5}, L"some_var"),
        Token(OP_ASSIGN, {3, 12}),
        Token(INT_LITERAL, {3, 14}, 2),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon
}

TEST_CASE("FunctionCall as an Instruction", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(IDENTIFIER, {3, 1}, L"f1"),
        Token(LPAREN, {3, 3}),
        Token(RPAREN, {3, 4}),
        Token(SEMICOLON, {3, 5}),
        Token(IDENTIFIER, {4, 1}, L"f2"),
        Token(LPAREN, {4, 3}),
        Token(STR_LITERAL, {4, 4}, L"1"),
        Token(RPAREN, {4, 7}),
        Token(SEMICOLON, {4, 8}),
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(LPAREN, {5, 3}),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(COMMA, {5, 10}),
        Token(FLOAT_LITERAL, {5, 12}, 1.2),
        Token(RPAREN, {5, 15}),
        Token(SEMICOLON, {5, 16}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  |-FunctionCall <line: 3, col: 1> functionName=f1\n"
                L"  |-FunctionCall <line: 4, col: 1> functionName=f2\n"
                L"  |`-Literal <line: 4, col: 4> value=\"1\"\n"
                L"  `-FunctionCall <line: 5, col: 1> functionName=f3\n"
                L"   |-Literal <line: 5, col: 4> value=1\n"
                L"   |-Literal <line: 5, col: 7> value=\"2\"\n"
                L"   `-Literal <line: 5, col: 12> value=1.2\n"
    );
}

TEST_CASE("FunctionCall as an Instruction errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(RPAREN, {5, 15}),
        Token(SEMICOLON, {5, 16}),
    });
    checkParseError<SyntaxError>(tokens); // missing left parenthesis
    tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(LPAREN, {5, 3}),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(RPAREN, {5, 15}),
        Token(SEMICOLON, {5, 16}),
    });
    checkParseError<SyntaxError>(tokens); // missing first parameter (parameter list begins with comma)
    tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(LPAREN, {5, 3}),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(RPAREN, {5, 15}),
        Token(SEMICOLON, {5, 16}),
    });
    checkParseError<SyntaxError>(tokens); // missing last parameter (parameter list ends with comma)
    tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(LPAREN, {5, 3}),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(SEMICOLON, {5, 16}),
    });
    checkParseError<SyntaxError>(tokens); // missing right parenthesis
    tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(LPAREN, {5, 3}),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(RPAREN, {5, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon
}
