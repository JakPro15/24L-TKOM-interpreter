#include "convertToString.hpp"

#include <codecvt>
#include <locale>

std::string convertToString(const std::wstring &string)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(string);
}
