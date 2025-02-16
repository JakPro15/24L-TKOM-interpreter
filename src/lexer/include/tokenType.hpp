#ifndef TOKENTYPE_HPP
#define TOKENTYPE_HPP

#include <format>
#include <ostream>
#include <sstream>

enum class TokenType
{
    KW_INCLUDE,
    KW_STRUCT,
    KW_VARIANT,
    KW_FUNC,
    KW_CONTINUE,
    KW_BREAK,
    KW_RETURN,
    KW_IF,
    KW_ELIF,
    KW_ELSE,
    KW_WHILE,
    KW_DO,
    KW_IS,
    KW_OR,
    KW_XOR,
    KW_AND,
    KW_NOT,
    KW_INT,
    KW_FLOAT,
    KW_BOOL,
    KW_STR,
    LBRACE,
    RBRACE,
    SEMICOLON,
    LPAREN,
    RPAREN,
    ARROW,
    COMMA,
    DOLLAR_SIGN,
    OP_ASSIGN,
    OP_DOT,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_IDENTICAL,
    OP_NOT_IDENTICAL,
    OP_CONCAT,
    OP_STR_MULTIPLY,
    OP_GREATER,
    OP_LESSER,
    OP_GREATER_EQUAL,
    OP_LESSER_EQUAL,
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_FLOOR_DIVIDE,
    OP_MODULO,
    OP_EXPONENT,
    LSQUAREBRACE,
    RSQUAREBRACE,
    IDENTIFIER,
    STR_LITERAL,
    INT_LITERAL,
    FLOAT_LITERAL,
    TRUE_LITERAL,
    FALSE_LITERAL,
    COMMENT,
    EOT
};

std::wstring toString(TokenType tokenType);
std::wostream &operator<<(std::wostream &out, TokenType tokenType);

template <>
struct std::formatter<TokenType, wchar_t>: std::formatter<std::wstring, wchar_t>
{
    template <class FormatContext>
    auto format(TokenType tokenType, FormatContext &context) const
    {
        return std::formatter<std::wstring, wchar_t>::format(toString(tokenType), context);
    }
};

#endif
