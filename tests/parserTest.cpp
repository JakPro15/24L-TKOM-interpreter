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
        Token(RBRACE, {20, 1}),
        Token(EOT, {20, 2}),
    };
    wrapped.insert(wrapped.begin() + 5, tokens.begin(), tokens.end());
    return wrapped;
}

std::vector<Token> wrapExpression(std::vector<Token> tokens)
{
    std::vector wrapped = {
        Token(IDENTIFIER, {3, 15}, L"print"),
        Token(LPAREN, {3, 25}),
        // here will be the expression
        Token(RPAREN, {20, 1}),
        Token(SEMICOLON, {20, 1}),
    };
    wrapped.insert(wrapped.begin() + 2, tokens.begin(), tokens.end());
    return wrapInFunction(wrapped);
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
        Token(IDENTIFIER, {4, 1}, L"f"),
        Token(LPAREN, {4, 3}),
        Token(RPAREN, {4, 4}),
        Token(SEMICOLON, {4, 5}),
        Token(RBRACE, {5, 1}),
        Token(EOT, {5, 2}),
    };
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function(str, typename): FunctionDeclaration <line: 1, col: 1> returnType=type_name\n"
                L" |-Parameters:\n"
                L" ||-VariableDeclaration <line: 1, col: 9> type=str name=param1 mutable=true\n"
                L" |`-VariableDeclaration <line: 1, col: 27> type=typename name=param2 mutable=false\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 4, col: 1> functionName=f\n"
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
                L"  ||-VariableDeclaration <line: 4, col: 1> type=int name=const_var mutable=false\n"
                L"  |`-Literal <line: 4, col: 22> type=int value=2\n"
                L"  `-VariableDeclStatement <line: 5, col: 1>\n"
                L"   |-VariableDeclaration <line: 5, col: 1> type=some_type name=mut_var mutable=true\n"
                L"   `-Literal <line: 5, col: 22> type=int value=3\n"
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
                L"  |`-Literal <line: 3, col: 14> type=int value=2\n"
                L"  `-AssignmentStatement <line: 4, col: 1>\n"
                L"   |-Assignable <line: 4, col: 1> right=some_var3\n"
                L"   |`-Assignable <line: 4, col: 1> right=some_var2\n"
                L"   | `-Assignable <line: 4, col: 1> right=some_var\n"
                L"   `-Literal <line: 4, col: 31> type=int value=3\n"
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
                L"  |`-Literal <line: 4, col: 4> type=string value=1\n"
                L"  `-FunctionCall <line: 5, col: 1> functionName=f3\n"
                L"   |-Literal <line: 5, col: 4> type=int value=1\n"
                L"   |-Literal <line: 5, col: 7> type=string value=2\n"
                L"   `-Literal <line: 5, col: 12> type=float value=1.2\n"
    );
}

TEST_CASE("FunctionCall errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(RPAREN, {5, 15}),
        Token(SEMICOLON, {5, 16}),
    });
    checkParseError<SyntaxError>(tokens); // missing left parenthesis - instruction case
    tokens = wrapExpression({
        Token(IDENTIFIER, {5, 1}, L"f3"),
        Token(INT_LITERAL, {5, 4}, 1),
        Token(COMMA, {5, 5}),
        Token(STR_LITERAL, {5, 7}, L"2"),
        Token(RPAREN, {5, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing left parenthesis - expression case
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

TEST_CASE("ContinueStatement, BreakStatement, ReturnStatement", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_CONTINUE, {4, 1}),
        Token(SEMICOLON, {4, 10}),
        Token(KW_BREAK, {5, 1}),
        Token(SEMICOLON, {5, 10}),
        Token(KW_RETURN, {6, 1}),
        Token(SEMICOLON, {6, 10}),
        Token(KW_RETURN, {7, 1}),
        Token(TRUE_LITERAL, {7, 10}),
        Token(SEMICOLON, {7, 15}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  |-ContinueStatement <line: 4, col: 1>\n"
                L"  |-BreakStatement <line: 5, col: 1>\n"
                L"  |-ReturnStatement <line: 6, col: 1>\n"
                L"  `-ReturnStatement <line: 7, col: 1>\n"
                L"   `-Literal <line: 7, col: 10> type=bool value=true\n"
    );
}

TEST_CASE("ContinueStatement, BreakStatement, ReturnStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_CONTINUE, {4, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon after continue
    tokens = wrapInFunction({
        Token(KW_BREAK, {5, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon after break
    tokens = wrapInFunction({
        Token(KW_RETURN, {6, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon after return
    tokens = wrapInFunction({
        Token(KW_RETURN, {7, 1}),
        Token(TRUE_LITERAL, {7, 10}),
    });
    checkParseError<SyntaxError>(tokens); // missing semicolon after return value
}

TEST_CASE("IfStatement - basic", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(IDENTIFIER, {6, 5}, L"called"),
        Token(LPAREN, {6, 15}),
        Token(RPAREN, {6, 16}),
        Token(SEMICOLON, {6, 17}),
        Token(RBRACE, {7, 1}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-IfStatement <line: 4, col: 1>\n"
                L"   `-SingleIfCase <line: 4, col: 1>\n"
                L"    |-Literal <line: 4, col: 10> type=int value=1\n"
                L"    `-FunctionCall <line: 6, col: 5> functionName=called\n"
    );
}

TEST_CASE("IfStatement - many cases", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_ELIF, {7, 3}),
        Token(LPAREN, {7, 8}),
        Token(IDENTIFIER, {7, 14}, L"type"),
        Token(IDENTIFIER, {7, 20}, L"name"),
        Token(OP_ASSIGN, {7, 26}),
        Token(IDENTIFIER, {7, 30}, L"variantVar"),
        Token(RPAREN, {7, 42}),
        Token(LBRACE, {7, 43}),
        Token(KW_RETURN, {7, 44}),
        Token(INT_LITERAL, {7, 52}, 0),
        Token(SEMICOLON, {7, 53}),
        Token(RBRACE, {7, 54}),
        Token(KW_ELIF, {8, 1}),
        Token(LPAREN, {8, 5}),
        Token(IDENTIFIER, {8, 6}, L"type2"),
        Token(DOLLAR_SIGN, {8, 12}),
        Token(IDENTIFIER, {8, 14}, L"name2"),
        Token(OP_ASSIGN, {8, 20}),
        Token(STR_LITERAL, {8, 22}, L""),
        Token(RPAREN, {8, 26}),
        Token(LBRACE, {8, 27}),
        Token(IDENTIFIER, {9, 5}, L"called1"),
        Token(LPAREN, {9, 15}),
        Token(RPAREN, {9, 16}),
        Token(SEMICOLON, {9, 17}),
        Token(IDENTIFIER, {10, 5}, L"called2"),
        Token(LPAREN, {10, 15}),
        Token(RPAREN, {10, 16}),
        Token(SEMICOLON, {10, 17}),
        Token(RBRACE, {11, 1}),
        Token(KW_ELSE, {12, 1}),
        Token(LBRACE, {13, 1}),
        Token(IDENTIFIER, {14, 5}, L"print"),
        Token(LPAREN, {14, 15}),
        Token(STR_LITERAL, {14, 16}, L"else case"),
        Token(RPAREN, {14, 30}),
        Token(SEMICOLON, {14, 31}),
        Token(RBRACE, {15, 1}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-IfStatement <line: 4, col: 1>\n"
                L"   |-SingleIfCase <line: 4, col: 1>\n"
                L"   |`-Literal <line: 4, col: 10> type=int value=1\n"
                L"   |-SingleIfCase <line: 7, col: 3>\n"
                L"   ||-VariableDeclStatement <line: 7, col: 14>\n"
                L"   |||-VariableDeclaration <line: 7, col: 14> type=type name=name mutable=false\n"
                L"   ||`-Variable <line: 7, col: 30> name=variantVar\n"
                L"   |`-ReturnStatement <line: 7, col: 44>\n"
                L"   | `-Literal <line: 7, col: 52> type=int value=0\n"
                L"   |-SingleIfCase <line: 8, col: 1>\n"
                L"   ||-VariableDeclStatement <line: 8, col: 6>\n"
                L"   |||-VariableDeclaration <line: 8, col: 6> type=type2 name=name2 mutable=true\n"
                L"   ||`-Literal <line: 8, col: 22> type=string value=\n"
                L"   ||-FunctionCall <line: 9, col: 5> functionName=called1\n"
                L"   |`-FunctionCall <line: 10, col: 5> functionName=called2\n"
                L"   `-ElseCase:\n"
                L"    `-FunctionCall <line: 14, col: 5> functionName=print\n"
                L"     `-Literal <line: 14, col: 16> type=string value=else case\n"
    );
}

TEST_CASE("IfStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition parentheses
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition right parenthesis
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing body
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_ELIF, {7, 3}),
        Token(LBRACE, {7, 43}),
        Token(RBRACE, {7, 54}),
    });
    checkParseError<SyntaxError>(tokens); // missing elif condition
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_ELIF, {7, 3}),
        Token(LPAREN, {7, 8}),
        Token(IDENTIFIER, {7, 30}, L"variantVar"),
        Token(RPAREN, {7, 42}),
    });
    checkParseError<SyntaxError>(tokens); // missing elif body
    tokens = wrapInFunction({
        Token(KW_IF, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_ELSE, {12, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing else body
}

TEST_CASE("WhileStatement", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_WHILE, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
        Token(LBRACE, {5, 1}),
        Token(IDENTIFIER, {6, 5}, L"called"),
        Token(LPAREN, {6, 15}),
        Token(RPAREN, {6, 16}),
        Token(SEMICOLON, {6, 17}),
        Token(RBRACE, {7, 1}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-WhileStatement <line: 4, col: 1>\n"
                L"   |-Literal <line: 4, col: 10> type=int value=1\n"
                L"   `-FunctionCall <line: 6, col: 5> functionName=called\n"
    );
}

TEST_CASE("WhileStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_WHILE, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition parentheses
    tokens = wrapInFunction({
        Token(KW_WHILE, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(RPAREN, {4, 15}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition
    tokens = wrapInFunction({
        Token(KW_WHILE, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition right parenthesis
    tokens = wrapInFunction({
        Token(KW_WHILE, {4, 1}),
        Token(LPAREN, {4, 5}),
        Token(INT_LITERAL, {4, 10}, 1),
        Token(RPAREN, {4, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing body
}

TEST_CASE("DoWhileStatement", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(IDENTIFIER, {6, 5}, L"called"),
        Token(LPAREN, {6, 15}),
        Token(RPAREN, {6, 16}),
        Token(SEMICOLON, {6, 17}),
        Token(RBRACE, {7, 1}),
        Token(KW_WHILE, {8, 1}),
        Token(LPAREN, {8, 5}),
        Token(INT_LITERAL, {8, 10}, 1),
        Token(RPAREN, {8, 15}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-DoWhileStatement <line: 4, col: 1>\n"
                L"   |-Literal <line: 8, col: 10> type=int value=1\n"
                L"   `-FunctionCall <line: 6, col: 5> functionName=called\n"
    );
}

TEST_CASE("DoWhileStatement errors", "[Parser]")
{
    std::vector tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(KW_WHILE, {8, 1}),
        Token(LPAREN, {8, 5}),
        Token(INT_LITERAL, {8, 10}, 1),
        Token(RPAREN, {8, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing body
    tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(LPAREN, {8, 5}),
        Token(INT_LITERAL, {8, 10}, 1),
        Token(RPAREN, {8, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing while
    tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_WHILE, {8, 1}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition parentheses
    tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_WHILE, {8, 1}),
        Token(LPAREN, {8, 5}),
        Token(RPAREN, {8, 15}),
    });
    checkParseError<SyntaxError>(tokens); // missing condition
    tokens = wrapInFunction({
        Token(KW_DO, {4, 1}),
        Token(LBRACE, {5, 1}),
        Token(RBRACE, {7, 1}),
        Token(KW_WHILE, {8, 1}),
        Token(LPAREN, {8, 5}),
        Token(INT_LITERAL, {8, 10}, 1),
    });
    checkParseError<SyntaxError>(tokens); // missing condition right parenthesis
}

void checkBinaryOperator(TokenType operatorType, std::wstring expressionName)
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(operatorType, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
    });
    checkParsing(
        tokens, std::format(
                    L"Program containing:\n"
                    L"Functions:\n"
                    L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                    L" `-Body:\n"
                    L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                    L"   `-{} <line: 4, col: 1>\n"
                    L"    |-Variable <line: 4, col: 1> name=a\n"
                    L"    `-Variable <line: 4, col: 6> name=b\n",
                    expressionName
                )
    );
}

void checkBinaryOpAssociativity(TokenType operatorType, std::wstring expressionName)
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(operatorType, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(operatorType, {4, 8}),
        Token(IDENTIFIER, {4, 11}, L"c"),
        Token(operatorType, {4, 13}),
        Token(IDENTIFIER, {4, 16}, L"d"),
    });
    checkParsing(
        tokens, std::format(
                    L"Program containing:\n"
                    L"Functions:\n"
                    L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                    L" `-Body:\n"
                    L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                    L"   `-{} <line: 4, col: 1>\n"
                    L"    |-{} <line: 4, col: 1>\n"
                    L"    ||-{} <line: 4, col: 1>\n"
                    L"    |||-Variable <line: 4, col: 1> name=a\n"
                    L"    ||`-Variable <line: 4, col: 6> name=b\n"
                    L"    |`-Variable <line: 4, col: 11> name=c\n"
                    L"    `-Variable <line: 4, col: 16> name=d\n",
                    expressionName, expressionName, expressionName
                )
    );
}

TEST_CASE("binary operators with associativity", "[Parser]")
{
    for(auto [operatorType, expressionName]: std::vector{
            std::pair{KW_OR, L"OrExpression"},
            {KW_XOR, L"XorExpression"},
            {KW_AND, L"AndExpression"},
            {OP_CONCAT, L"ConcatExpression"},
            {OP_STR_MULTIPLY, L"StringMultiplyExpression"},
            {OP_PLUS, L"PlusExpression"},
            {OP_MINUS, L"MinusExpression"},
            {OP_MULTIPLY, L"MultiplyExpression"},
            {OP_DIVIDE, L"DivideExpression"},
            {OP_FLOOR_DIVIDE, L"FloorDivideExpression"},
            {OP_MODULO, L"ModuloExpression"},
            {OP_EXPONENT, L"ExponentExpression"}
        })
    {
        checkBinaryOperator(operatorType, expressionName);
        checkBinaryOpAssociativity(operatorType, expressionName);
    }
}

void checkBinaryOpErrors(TokenType operatorType)
{
    std::vector tokens = wrapExpression({
        Token(operatorType, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
    });
    checkParseError<SyntaxError>(tokens); // first expression missing
    tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(operatorType, {4, 3}),
    });
    checkParseError<SyntaxError>(tokens); // second expression missing
}

void checkBinaryOpNonAssociativity(TokenType operatorType)
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(operatorType, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(operatorType, {4, 8}),
        Token(IDENTIFIER, {4, 6}, L"b"),
    });
    checkParseError<SyntaxError>(tokens); // multiple operators in a row invalid
}

TEST_CASE("binary operators with associativity errors", "[Parser]")
{
    for(TokenType operatorType:
        {KW_OR, KW_XOR, KW_AND, OP_CONCAT, OP_STR_MULTIPLY, OP_PLUS, OP_MULTIPLY, OP_DIVIDE, OP_FLOOR_DIVIDE, OP_MODULO,
         OP_EXPONENT})
    {
        checkBinaryOpErrors(operatorType);
    }
}

TEST_CASE("non-associative binary operators", "[Parser]")
{
    for(auto [operatorType, expressionName]: std::vector{
            std::pair{OP_EQUAL, L"EqualExpression"},
            {OP_NOT_EQUAL, L"NotEqualExpression"},
            {OP_IDENTICAL, L"IdenticalExpression"},
            {OP_NOT_IDENTICAL, L"NotIdenticalExpression"},
            {OP_GREATER, L"GreaterExpression"},
            {OP_LESSER, L"LesserExpression"},
            {OP_GREATER_EQUAL, L"GreaterEqualExpression"},
            {OP_LESSER_EQUAL, L"LesserEqualExpression"}
        })
    {
        checkBinaryOperator(operatorType, expressionName);
    }
}

TEST_CASE("non-associative binary operators errors", "[Parser]")
{
    for(TokenType operatorType:
        {OP_EQUAL, OP_NOT_EQUAL, OP_IDENTICAL, OP_NOT_IDENTICAL, OP_GREATER, OP_LESSER, OP_GREATER_EQUAL,
         OP_LESSER_EQUAL})
    {
        checkBinaryOpErrors(operatorType);
        checkBinaryOpNonAssociativity(operatorType);
    }
}

TEST_CASE("MinusExpression and UnaryMinusExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(OP_MINUS, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
        Token(OP_MINUS, {4, 3}),
        Token(OP_MINUS, {4, 4}),
        Token(IDENTIFIER, {4, 5}, L"b"),
        Token(OP_MINUS, {4, 6}),
        Token(OP_MINUS, {4, 7}),
        Token(IDENTIFIER, {4, 8}, L"c"),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   `-MinusExpression <line: 4, col: 1>\n"
                L"    |-MinusExpression <line: 4, col: 1>\n"
                L"    ||-UnaryMinusExpression <line: 4, col: 1>\n"
                L"    ||`-Variable <line: 4, col: 2> name=a\n"
                L"    |`-UnaryMinusExpression <line: 4, col: 4>\n"
                L"    | `-Variable <line: 4, col: 5> name=b\n"
                L"    `-UnaryMinusExpression <line: 4, col: 7>\n"
                L"     `-Variable <line: 4, col: 8> name=c\n"

    );
}

TEST_CASE("NotExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(KW_NOT, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   `-NotExpression <line: 4, col: 1>\n"
                L"    `-Variable <line: 4, col: 2> name=a\n"
    );
}

TEST_CASE("unary operators errors", "[Parser]")
{
    for(TokenType operatorType: {OP_MINUS, KW_NOT})
    {
        std::vector tokens = wrapExpression({
            Token(operatorType, {4, 1}),
        });
        checkParseError<SyntaxError>(tokens); // expression after unary operator missing
    }
}

TEST_CASE("IsExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(KW_IS, {4, 3}),
        Token(KW_INT, {4, 6}),
        Token(COMMA, {4, 9}),
        Token(IDENTIFIER, {4, 10}, L"a"),
        Token(KW_IS, {4, 12}),
        Token(IDENTIFIER, {4, 15}, L"type"),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   |-IsExpression <line: 4, col: 1> right=int\n"
                L"   |`-Variable <line: 4, col: 1> name=a\n"
                L"   `-IsExpression <line: 4, col: 10> right=type\n"
                L"    `-Variable <line: 4, col: 10> name=a\n"
    );
}

TEST_CASE("IsExpression errors", "[Parser]")
{
    checkBinaryOpErrors(KW_IS);
    checkBinaryOpNonAssociativity(KW_IS);

    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(KW_IS, {4, 3}),
        Token(INT_LITERAL, {4, 10}, 2),
    });
    checkParseError<SyntaxError>(tokens);
}

TEST_CASE("SubscriptExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(LSQUAREBRACE, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(LSQUAREBRACE, {4, 9}),
        Token(IDENTIFIER, {4, 12}, L"c"),
        Token(RSQUAREBRACE, {4, 15}),
        Token(RSQUAREBRACE, {4, 18}),
        Token(LSQUAREBRACE, {4, 21}),
        Token(INT_LITERAL, {4, 24}, 2),
        Token(RSQUAREBRACE, {4, 27}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   `-SubscriptExpression <line: 4, col: 1>\n"
                L"    |-SubscriptExpression <line: 4, col: 1>\n"
                L"    ||-Variable <line: 4, col: 1> name=a\n"
                L"    |`-SubscriptExpression <line: 4, col: 6>\n"
                L"    | |-Variable <line: 4, col: 6> name=b\n"
                L"    | `-Variable <line: 4, col: 12> name=c\n"
                L"    `-Literal <line: 4, col: 24> type=int value=2\n"
    );
}

TEST_CASE("SubscriptExpression errors", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(LSQUAREBRACE, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(RSQUAREBRACE, {4, 12}),
    });
    checkParseError<SyntaxError>(tokens); // first expression missing
    tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(LSQUAREBRACE, {4, 3}),
        Token(RSQUAREBRACE, {4, 12}),
    });
    checkParseError<SyntaxError>(tokens); // second expression missing
    tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(RSQUAREBRACE, {4, 12}),
    });
    checkParseError<SyntaxError>(tokens); // left bracket missing
    tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(LSQUAREBRACE, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
    });
    checkParseError<SyntaxError>(tokens); // right bracket missing
}

TEST_CASE("DotExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(OP_DOT, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
        Token(OP_DOT, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"c"),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   `-DotExpression <line: 4, col: 1> field=c\n"
                L"    `-DotExpression <line: 4, col: 1> field=b\n"
                L"     `-Variable <line: 4, col: 1> name=a\n"
    );
}

TEST_CASE("DotExpression errors", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(OP_DOT, {4, 3}),
        Token(IDENTIFIER, {4, 6}, L"b"),
    });
    checkParseError<SyntaxError>(tokens); // first expression missing
    tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"a"),
        Token(OP_DOT, {4, 3}),
        Token(STR_LITERAL, {4, 6}, L"b"),
    });
    checkParseError<SyntaxError>(tokens); // non-identifier after .
}

TEST_CASE("StructExpression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(LBRACE, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
        Token(RBRACE, {4, 3}),
        Token(COMMA, {4, 4}),
        Token(LBRACE, {5, 1}),
        Token(IDENTIFIER, {5, 2}, L"a"),
        Token(COMMA, {5, 3}),
        Token(IDENTIFIER, {5, 4}, L"b"),
        Token(COMMA, {5, 5}),
        Token(IDENTIFIER, {5, 6}, L"c"),
        Token(RBRACE, {5, 7}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   |-StructExpression <line: 4, col: 1>\n"
                L"   |`-Variable <line: 4, col: 2> name=a\n"
                L"   `-StructExpression <line: 5, col: 1>\n"
                L"    |-Variable <line: 5, col: 2> name=a\n"
                L"    |-Variable <line: 5, col: 4> name=b\n"
                L"    `-Variable <line: 5, col: 6> name=c\n"
    );
}

TEST_CASE("StructExpression errors", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(LBRACE, {4, 1}),
        Token(RBRACE, {4, 3}),
    });
    checkParseError<SyntaxError>(tokens); // there needs to be at least 1 parameter
    tokens = wrapExpression({
        Token(LBRACE, {5, 1}),
        Token(IDENTIFIER, {5, 2}, L"a"),
        Token(IDENTIFIER, {5, 4}, L"b"),
        Token(RBRACE, {5, 7}),
    });
    checkParseError<SyntaxError>(tokens); // comma missing
    tokens = wrapExpression({
        Token(LBRACE, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
        Token(COMMA, {4, 4}),
        Token(RBRACE, {4, 3}),
    });
    checkParseError<SyntaxError>(tokens); // trailing comma
    tokens = wrapExpression({
        Token(LBRACE, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
    });
    checkParseError<SyntaxError>(tokens); // closing brace missing
}

TEST_CASE("FunctionCall as an expression", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(IDENTIFIER, {4, 1}, L"f1"),
        Token(LPAREN, {4, 3}),
        Token(RPAREN, {4, 4}),
        Token(COMMA, {4, 5}),
        Token(IDENTIFIER, {5, 1}, L"f2"),
        Token(LPAREN, {5, 3}),
        Token(STR_LITERAL, {5, 4}, L"1"),
        Token(RPAREN, {5, 7}),
        Token(COMMA, {5, 8}),
        Token(IDENTIFIER, {6, 1}, L"f3"),
        Token(LPAREN, {6, 3}),
        Token(INT_LITERAL, {6, 4}, 1),
        Token(COMMA, {6, 5}),
        Token(STR_LITERAL, {6, 7}, L"2"),
        Token(COMMA, {6, 10}),
        Token(FLOAT_LITERAL, {6, 12}, 1.2),
        Token(RPAREN, {6, 15}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   |-FunctionCall <line: 4, col: 1> functionName=f1\n"
                L"   |-FunctionCall <line: 5, col: 1> functionName=f2\n"
                L"   |`-Literal <line: 5, col: 4> type=string value=1\n"
                L"   `-FunctionCall <line: 6, col: 1> functionName=f3\n"
                L"    |-Literal <line: 6, col: 4> type=int value=1\n"
                L"    |-Literal <line: 6, col: 7> type=string value=2\n"
                L"    `-Literal <line: 6, col: 12> type=float value=1.2\n"
    );
}

TEST_CASE("Expression in parentheses", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(LPAREN, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
        Token(RPAREN, {4, 3}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   `-Variable <line: 4, col: 2> name=a\n"
    );
}

TEST_CASE("Expression in parentheses errors", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(LPAREN, {4, 1}),
        Token(RPAREN, {4, 3}),
    });
    checkParseError<SyntaxError>(tokens); // missing expression
    tokens = wrapExpression({
        Token(LPAREN, {4, 1}),
        Token(IDENTIFIER, {4, 2}, L"a"),
    });
    checkParseError<SyntaxError>(tokens); // missing closing parenthesis
}

TEST_CASE("Literal", "[Parser]")
{
    std::vector tokens = wrapExpression({
        Token(INT_LITERAL, {4, 1}, 2),
        Token(COMMA, {4, 2}),
        Token(STR_LITERAL, {4, 3}, L"2"),
        Token(COMMA, {4, 6}),
        Token(FLOAT_LITERAL, {4, 7}, 2.0),
        Token(COMMA, {4, 10}),
        Token(TRUE_LITERAL, {4, 11}),
        Token(COMMA, {4, 16}),
        Token(FALSE_LITERAL, {4, 17}),
    });
    checkParsing(
        tokens, L"Program containing:\n"
                L"Functions:\n"
                L"`-a_function: FunctionDeclaration <line: 1, col: 1>\n"
                L" `-Body:\n"
                L"  `-FunctionCall <line: 3, col: 15> functionName=print\n"
                L"   |-Literal <line: 4, col: 1> type=int value=2\n"
                L"   |-Literal <line: 4, col: 3> type=string value=2\n"
                L"   |-Literal <line: 4, col: 7> type=float value=2\n"
                L"   |-Literal <line: 4, col: 11> type=bool value=true\n"
                L"   `-Literal <line: 4, col: 17> type=bool value=false\n"
    );
}
