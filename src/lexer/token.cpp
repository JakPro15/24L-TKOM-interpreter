#include "token.hpp"

Token::Token(TokenType type, Position position): type(type), position(position) {}

Token::Token(TokenType type, Position position, std::variant<std::wstring, int32_t, double> value):
    type(type), position(position), value(value)
{}
