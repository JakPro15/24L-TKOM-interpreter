#include "lexerExceptions.hpp"

#include "convertToString.hpp"

InvalidTokenValueError::InvalidTokenValueError(std::wstring message): std::runtime_error(convertToString(message)) {}
