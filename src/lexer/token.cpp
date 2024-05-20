#include "token.hpp"

#include "lexerExceptions.hpp"

#include <array>
#include <iterator>
#include <ostream>

using enum TokenType;

Token::Token(TokenType type, Position position): type(type), position(position)
{
    if(type == IDENTIFIER || type == COMMENT || type == STR_LITERAL || type == INT_LITERAL || type == FLOAT_LITERAL)
        throw InvalidTokenValueError(std::format(L"A value needs to be given for a token of type {}", type));
}

Token::Token(TokenType type, Position position, std::wstring value): type(type), position(position), value(value)
{
    if(type != IDENTIFIER && type != COMMENT && type != STR_LITERAL)
        throw InvalidTokenValueError(L"Only identifier and comment tokens can contain std::wstring value");
}

Token::Token(TokenType type, Position position, int32_t value): type(type), position(position), value(value)
{
    if(type != INT_LITERAL)
        throw InvalidTokenValueError(L"Only integer literal tokens can contain int32_t value");
}

Token::Token(TokenType type, Position position, double value): type(type), position(position), value(value)
{
    if(type != FLOAT_LITERAL)
        throw InvalidTokenValueError(L"Only float literal tokens can contain double value");
}

TokenType Token::getType() const
{
    return type;
}

Position Token::getPosition() const
{
    return position;
}

std::variant<std::monostate, std::wstring, int32_t, double> Token::getValue() const
{
    return value;
}

std::wostream &operator<<(std::wostream &out, Token token)
{
    std::ostream_iterator<wchar_t, wchar_t> outIterator(out);
    std::format_to(outIterator, L"{}", token);
    return out;
}
