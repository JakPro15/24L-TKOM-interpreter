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

template <class CharT>
struct std::formatter<Token, CharT>: std::formatter<std::wstring, CharT>
{
    template <class FormatContext>
    auto format(Token token, FormatContext &context) const
    {
        std::wstringstream out;
        out << token;
        return std::formatter<std::wstring, CharT>::format(out.str(), context);
    }
};

#endif
