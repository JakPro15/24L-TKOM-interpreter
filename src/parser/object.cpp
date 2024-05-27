#include "object.hpp"

Object::Object(
    Type type, std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> value
): type(type), value(std::move(value))
{}

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
