#ifndef APPEXCEPTIONS_HPP
#define APPEXCEPTIONS_HPP

#include <stdexcept>

class AppError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class FileError: public AppError
{
    using AppError::AppError;
};

class DuplicateFileError: public AppError
{
    using AppError::AppError;
};

class NoFilesError: public AppError
{
    using AppError::AppError;
};

#endif
