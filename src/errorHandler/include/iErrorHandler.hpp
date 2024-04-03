#include "position.hpp"

#include <string>

class IErrorHandler
{
public:
    virtual void handleError(std::wstring message, Position position) = 0;
};
