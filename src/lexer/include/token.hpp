#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "position.hpp"
#include "tokenType.hpp"

#include <cstdint>
#include <format>
#include <sstream>
#include <string>
#include <variant>

class Token
{
public:
    Token(TokenType type, Position position);
    Token(TokenType type, Position position, std::wstring value);
    Token(TokenType type, Position position, int32_t value);
    Token(TokenType type, Position position, double value);

    TokenType getType() const;
    Position getPosition() const;
    std::variant<std::monostate, std::wstring, int32_t, double> getValue() const;
    bool operator==(const Token &) const = default;
    friend std::wostream &operator<<(std::wostream &out, Token token);
private:
    TokenType type;
    Position position;
    std::variant<std::monostate, std::wstring, int32_t, double> value;
};

template <>
struct std::formatter<Token, wchar_t>: std::formatter<std::wstring, wchar_t>
{
    template <class FormatContext>
    auto format(Token token, FormatContext &context) const
    {
        using enum TokenType;
        switch(token.getType())
        {
        case IDENTIFIER:
            return std::format_to(context.out(), L"{}", std::get<std::wstring>(token.getValue()));
        case STR_LITERAL:
            return std::format_to(context.out(), L"{}", std::get<std::wstring>(token.getValue()));
        case INT_LITERAL:
            return std::format_to(context.out(), L"{}", std::get<int32_t>(token.getValue()));
        case FLOAT_LITERAL:
            return std::format_to(context.out(), L"{}", std::get<double>(token.getValue()));
        case COMMENT:
            return std::format_to(context.out(), L"#{}", std::get<std::wstring>(token.getValue()));
        default:
            return std::format_to(context.out(), L"{}", token.getType());
        }
    }
};

#endif
