#include "position.hpp"

#include <string>

enum class Error
{
    READER_CONTROL_CHAR,
    READER_INPUT_ERROR,
    LEXER_IDENTIFIER_TOO_LONG,
    LEXER_COMMENT_TOO_LONG,
};

class IErrorHandler
{
public:
    [[noreturn]]
    virtual void handleError(Error error, std::wstring message, Position position) = 0;
};
