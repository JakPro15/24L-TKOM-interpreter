#ifndef READEREXCEPTIONS_HPP
#define READEREXCEPTIONS_HPP

#include "position.hpp"

#include <stdexcept>

class ErrorAtPosition: public std::exception
{
public:
    ErrorAtPosition(std::wstring message, Position position);
    const char *what() const noexcept override;
    std::wstring getMessage() const;
    Position getPosition() const;
private:
    std::string whatMessage;
    std::wstring message;
    Position position;
};

class ReaderException: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_READER_ERROR(class_name)        \
    class class_name: public ReaderException    \
    {                                           \
        using ReaderException::ReaderException; \
    };

DECLARE_READER_ERROR(ControlCharError);
DECLARE_READER_ERROR(ReaderInputError);

#endif
