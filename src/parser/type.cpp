#include "type.hpp"

#include <iterator>
#include <ostream>

bool Type::isBuiltin() const
{
    return std::holds_alternative<Type::Builtin>(value);
}

bool Type::isInitList() const
{
    return std::holds_alternative<Type::InitializationList>(value);
}

std::wostream &operator<<(std::wostream &out, Type type)
{
    std::ostream_iterator<wchar_t, wchar_t> outIterator(out);
    std::format_to(outIterator, L"{}", type);
    return out;
}
