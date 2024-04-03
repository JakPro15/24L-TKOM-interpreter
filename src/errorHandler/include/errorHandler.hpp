#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "iErrorHandler.hpp"

#include <iostream>

class ErrorHandler: public IErrorHandler
{
public:
    ErrorHandler(std::wostream &output = std::wcerr);
    [[noreturn]]
    void handleError(Error error, std::wstring message, Position position) override;
private:
    std::wostream &output;
};

#endif
