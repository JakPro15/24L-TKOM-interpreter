#ifndef TESTERRORHANDLER_HPP
#define TESTERRORHANDLER_HPP

#include "iErrorHandler.hpp"

#include <iostream>

class TestErrorHandler: public IErrorHandler
{
public:
    Error error;
    std::wstring message;
    Position position;

    [[noreturn]]
    void handleError(Error error, std::wstring message, Position position) override
    {
        this->error = error;
        std::wcerr << message << std::endl;
        this->position = position;
        throw std::exception();
    }
};

#endif
