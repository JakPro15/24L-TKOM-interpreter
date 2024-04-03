#ifndef IERRORHANDLER_HPP
#define IERRORHANDLER_HPP

#include "position.hpp"

#include <string>

enum class Error
{
    READER_CONTROL_CHAR,
    READER_INPUT_ERROR,
    LEXER_IDENTIFIER_TOO_LONG,
    LEXER_COMMENT_TOO_LONG,
    LEXER_STRING_TOO_LONG,
    LEXER_NEWLINE_IN_STRING,
    LEXER_INVALID_HEX_CHAR,
    LEXER_UNKNOWN_ESCAPE,
    LEXER_INT_TOO_LARGE,
    LEXER_UNKNOWN_TOKEN,
};

class IErrorHandler
{
public:
    [[noreturn]]
    virtual void handleError(Error error, std::wstring message, Position position) = 0;
};

#endif
