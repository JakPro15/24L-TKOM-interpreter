#include "type.hpp"

#include <memory>
#include <variant>

struct Object
{
    Type type;
    std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> value;
    bool operator==(const Object &other) const = default;
};
