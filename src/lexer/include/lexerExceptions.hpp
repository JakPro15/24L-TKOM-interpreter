#ifndef LEXEREXCEPTIONS_HPP
#define LEXEREXCEPTIONS_HPP

#include "readerExceptions.hpp"

class LexerException: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_LEXER_ERROR(class_name)       \
    class class_name: public LexerException   \
    {                                         \
        using LexerException::LexerException; \
    };

DECLARE_LEXER_ERROR(IdentifierTooLongError);
DECLARE_LEXER_ERROR(CommentTooLongError);
DECLARE_LEXER_ERROR(InvalidHexCharError);
DECLARE_LEXER_ERROR(UnknownEscapeSequenceError);
DECLARE_LEXER_ERROR(NewlineInStringError);
DECLARE_LEXER_ERROR(UnterminatedStringError);
DECLARE_LEXER_ERROR(StringTooLongError);
DECLARE_LEXER_ERROR(IntWithLeadingZeroError);
DECLARE_LEXER_ERROR(IntTooLargeError);
DECLARE_LEXER_ERROR(InvalidExponentError);
DECLARE_LEXER_ERROR(UnknownTokenError);

class InvalidTokenValueError: public std::runtime_error
{
public:
    InvalidTokenValueError(std::wstring message);
};

#endif
