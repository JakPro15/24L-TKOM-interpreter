#include "token.hpp"

#include "lexerExceptions.hpp"

#include <array>
#include <ostream>

using enum TokenType;

Token::Token(TokenType type, Position position): type(type), position(position)
{
    if(type == IDENTIFIER || type == COMMENT || type == STR_LITERAL || type == INT_LITERAL || type == FLOAT_LITERAL)
        throw InvalidTokenValueError(L"A value needs to be given for this type of token", position);
}

Token::Token(TokenType type, Position position, std::wstring value): type(type), position(position), value(value)
{
    if(type != IDENTIFIER && type != COMMENT && type != STR_LITERAL)
        throw InvalidTokenValueError(L"Only identifier and comment tokens can contain std::wstring value", position);
}

Token::Token(TokenType type, Position position, int32_t value): type(type), position(position), value(value)
{
    if(type != INT_LITERAL)
        throw InvalidTokenValueError(L"Only integer literal tokens can contain int32_t value", position);
}

Token::Token(TokenType type, Position position, double value): type(type), position(position), value(value)
{
    if(type != FLOAT_LITERAL)
        throw InvalidTokenValueError(L"Only float literal tokens can contain double value", position);
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
    switch(token.type)
    {
    case IDENTIFIER:
        out << std::get<std::wstring>(token.getValue());
        break;
    case STR_LITERAL:
        out << std::get<std::wstring>(token.getValue());
        break;
    case INT_LITERAL:
        out << std::get<int32_t>(token.getValue());
        break;
    case FLOAT_LITERAL:
        out << std::get<double>(token.getValue());
        break;
    case COMMENT:
        out << L"#" << std::get<std::wstring>(token.getValue());
        break;
    default:
        out << token.type;
    }
    return out;
}
