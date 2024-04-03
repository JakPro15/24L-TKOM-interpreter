#include "errorHandler.hpp"

ErrorHandler::ErrorHandler(std::wostream &output): output(output) {}

void ErrorHandler::handleError(Error error, std::wstring message, Position position)
{
    output << "Error: " << message << "\nat line " << position.line << ", column " << position.column << " of input."
           << std::endl;
    std::exit(1);
}
