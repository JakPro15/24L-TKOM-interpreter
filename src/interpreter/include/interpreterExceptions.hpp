#ifndef INTERPRETEREXCEPTIONS_HPP
#define INTERPRETEREXCEPTIONS_HPP

#include "documentTree.hpp"
#include "readerExceptions.hpp"

#include <string>
#include <vector>

class SemanticError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class FileError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class CircularIncludeError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#endif
