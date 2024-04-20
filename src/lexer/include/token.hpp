#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "position.hpp"
#include "tokenType.hpp"

#include <cstdint>
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
private:
    TokenType type;
    Position position;
    std::variant<std::monostate, std::wstring, int32_t, double> value;
};

#endif
