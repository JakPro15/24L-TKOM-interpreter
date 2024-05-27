#include "type.hpp"

#include <variant>

struct Object
{
    Type type;
    std::variant<std::wstring, int32_t, double, bool, std::vector<Object>> value;
};
