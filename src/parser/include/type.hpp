#ifndef TYPE_HPP
#define TYPE_HPP

#include <algorithm>
#include <format>
#include <variant>
#include <vector>

struct Type
{
    enum class Builtin
    {
        INT,
        FLOAT,
        STR,
        BOOL
    };
    typedef std::vector<Type> InitializationList;
    std::variant<Builtin, std::wstring, InitializationList> value;
    bool operator==(const Type &other) const = default;
    bool isBuiltin() const;
    bool isInitList() const;
};

std::wostream &operator<<(std::wostream &out, Type type);

template <>
struct std::formatter<Type::Builtin, wchar_t>: std::formatter<std::wstring, wchar_t>
{
    template <class FormatContext>
    auto format(Type::Builtin type, FormatContext &context) const
    {
        using enum Type::Builtin;
        std::wstring formatted;
        switch(type)
        {
        case INT:
            formatted = L"int";
            break;
        case FLOAT:
            formatted = L"float";
            break;
        case STR:
            formatted = L"str";
            break;
        case BOOL:
            formatted = L"bool";
            break;
        default:
            formatted = L"unknown";
        }
        return std::formatter<std::wstring, wchar_t>::format(formatted, context);
    }
};

template <>
struct std::formatter<Type::InitializationList, wchar_t>: std::formatter<std::wstring, wchar_t>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext &context)
    {
        if(context.begin() != context.end() && *context.begin() != L'}')
            throw std::format_error("std::vector<Type> does not take any format args.");
        return context.begin();
    }

    template <class FormatContext>
    auto format(const Type::InitializationList &types, FormatContext &context) const
    {
        std::format_to(context.out(), L"{{");
        if(!types.empty())
        {
            std::format_to(context.out(), L"{}", types[0]);
            std::for_each(types.begin() + 1, types.end(), [&](const Type &type) {
                std::format_to(context.out(), L", {}", type);
            });
        }
        return std::format_to(context.out(), L"}}");
    }
};

template <>
struct std::formatter<Type, wchar_t>: std::formatter<std::wstring, wchar_t>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext &context)
    {
        if(context.begin() != context.end() && *context.begin() != L'}')
            throw std::format_error("Type does not take any format args.");
        return context.begin();
    }

    template <class FormatContext>
    auto format(const Type &type, FormatContext &context) const
    {
        std::visit([&](auto value) { std::format_to(context.out(), L"{}", value); }, type.value);
        return context.out();
    }
};

template <>
struct std::hash<Type>
{
    std::size_t operator()(const Type &type) const
    {
        if(type.isBuiltin())
            return static_cast<std::size_t>(std::get<Type::Builtin>(type.value));
        else
            return std::hash<std::wstring>()(std::get<std::wstring>(type.value));
    }
};

#endif
