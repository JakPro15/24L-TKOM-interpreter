#include "object.hpp"

Object::Object(
    Type type, std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> value
): type(type), value(std::move(value))
{}

bool Object::operator==(const Object &other) const
{
    if(type != other.type)
        return false;
    if(std::holds_alternative<std::unique_ptr<Object>>(value))
    {
        if(!std::holds_alternative<std::unique_ptr<Object>>(other.value))
            return false;
        return *std::get<std::unique_ptr<Object>>(value) == *std::get<std::unique_ptr<Object>>(other.value);
    }
    return value == other.value;
}

bool Object::operator!=(const Object &other) const
{
    return !(*this == other);
}

namespace {
template <typename Contained>
std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> copyValue(
    const Contained &value
)
{
    return value;
}

template <>
std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> copyValue(
    const std::unique_ptr<Object> &value
)
{
    return std::make_unique<Object>(Object(*value.get()));
}
}

Object::Object(const Object &other):
    type(other.type), value(std::visit([](const auto &value) { return copyValue(value); }, other.value))
{}
