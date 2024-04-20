#include "commentDiscarder.hpp"

#include "fakeLexer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
using enum TokenType;

TEST_CASE("no comments", "[CommentDiscarder]")
{
    std::array tokens = {
        Token(KW_INT, {1, 1}),     Token(IDENTIFIER, {1, 10}, L"iden"),
        Token(OP_ASSIGN, {1, 20}), Token(STR_LITERAL, {1, 30}, L"iden"),
        Token(SEMICOLON, {1, 30}), Token(EOT, {2, 1}),
    };
    FakeLexer lexer(tokens);
    CommentDiscarder discarder(lexer);
    for(auto token: tokens)
        REQUIRE(discarder.getNextToken() == token);
}

TEST_CASE("skipping comments", "[CommentDiscarder]")
{
    std::array tokens = {
        Token(KW_INT, {1, 1}),
        Token(IDENTIFIER, {1, 10}, L"iden"),
        Token(COMMENT, {1, 30}, L"first comment"),
        Token(OP_ASSIGN, {1, 20}),
        Token(STR_LITERAL, {1, 30}, L"iden"),
        Token(COMMENT, {1, 30}, L"a comment"),
        Token(COMMENT, {1, 30}, L"another comment"),
        Token(EOT, {2, 1}),
    };
    FakeLexer lexer(tokens);
    CommentDiscarder discarder(lexer);
    for(auto token: tokens)
        if(token.getType() != COMMENT)
            REQUIRE(discarder.getNextToken() == token);
}
