#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "position.hpp"
#include "tokenType.hpp"

#include <cstdint>
#include <string>
#include <variant>

struct Token
{
    TokenType type;
    std::variant<std::wstring, int32_t, double> value;
    Position position;
};

#endif
