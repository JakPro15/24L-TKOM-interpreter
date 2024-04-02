#include "lexer.hpp"

#include "token.hpp"

#include <string>
#include <unordered_map>

const std::unordered_map<std::wstring, TokenType> keywordToTokenType = {
    {L"include", TokenType::KW_INCLUDE},
    {L"struct", TokenType::KW_STRUCT},
    {L"variant", TokenType::KW_VARIANT},
    {L"func", TokenType::KW_FUNC},
    {L"continue", TokenType::KW_CONTINUE},
    {L"break", TokenType::KW_BREAK},
    {L"return", TokenType::KW_RETURN},
    {L"if", TokenType::KW_IF},
    {L"elif", TokenType::KW_ELIF},
    {L"else", TokenType::KW_ELSE},
    {L"while", TokenType::KW_WHILE},
    {L"do", TokenType::KW_DO},
    {L"is", TokenType::KW_IS},
    {L"or", TokenType::KW_OR},
    {L"xor", TokenType::KW_XOR},
    {L"and", TokenType::KW_AND},
    {L"not", TokenType::KW_NOT},
    {L"int", TokenType::KW_INT},
    {L"float", TokenType::KW_FLOAT},
    {L"bool", TokenType::KW_BOOL},
    {L"str", TokenType::KW_STR},
};

Token Lexer::getNextToken()
{
    return Token();
}
