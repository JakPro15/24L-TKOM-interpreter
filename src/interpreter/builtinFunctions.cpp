#include "builtinFunctions.hpp"

#include "runtimeExceptions.hpp"

#include <limits>

using enum Type::Builtin;

BuiltinFunction builtinNoArguments(const std::vector<std::wstring> &arguments)
{
    return {
        FunctionIdentification(L"no_arguments", {}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {}, {{INT}},
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>>) -> std::optional<Object> {
                size_t noArguments = arguments.size();
                if(noArguments > std::numeric_limits<int32_t>::max())
                    throw RuntimeError(
                        L"Program arguments number exceeds int type maximum value", callSource, callPosition
                    ); // *should* not be reachable
                return Object{{INT}, static_cast<int32_t>(arguments.size())};
            }
        )
    };
}

template <typename T>
T getArg(const std::vector<std::reference_wrapper<Object>> &args)
{
    return std::get<T>(args[0].get().value);
}

BuiltinFunction builtinArgument(const std::vector<std::wstring> &arguments)
{
    return {
        FunctionIdentification(L"argument", {{INT}}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {INT}, L"index", false)}, {{STR}},
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
                auto index = getArg<int32_t>(args);
                if(index < 0 || static_cast<size_t>(index) >= arguments.size())
                    throw BuiltinFunctionArgumentError(
                        std::format(L"There is no program argument with index {}", index), callSource, callPosition
                    );
                return Object{{STR}, arguments[index]};
            }
        )
    };
}

BuiltinFunction builtinPrint(std::wostream &output)
{
    return {
        FunctionIdentification(L"print", {{STR}}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {STR}, L"message", false)}, std::nullopt,
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
                auto message = getArg<std::wstring>(args);
                output << message;
                if(output.bad())
                    throw StandardOutputError(L"Standard output stream returned error", callSource, callPosition);
                return std::nullopt;
            }
        )
    };
}

BuiltinFunction builtinPrintln(std::wostream &output)
{
    return {
        FunctionIdentification(L"println", {{STR}}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {STR}, L"message", false)}, std::nullopt,
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
                auto message = getArg<std::wstring>(args);
                output << message << L'\n';
                if(output.bad())
                    throw StandardOutputError(L"Standard output stream returned error", callSource, callPosition);
                return std::nullopt;
            }
        )
    };
}

BuiltinFunction builtinInputLine(std::wistream &input)
{
    return {
        FunctionIdentification(L"input", {}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {}, {{STR}},
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>>) -> std::optional<Object> {
                std::wstring line;
                std::getline(input, line);
                if(input.bad())
                    throw StandardInputError(L"Standard input stream returned error", callSource, callPosition);
                return Object{{STR}, line};
            }
        )
    };
}

BuiltinFunction builtinInput(std::wistream &input)
{
    return {
        FunctionIdentification(L"input", {{INT}}),
        BuiltinFunctionDeclaration(
            {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {INT}, L"no_chars", false)}, {{STR}},
            [&](Position callPosition, const std::wstring &callSource,
                std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
                auto numberOfCharacters = getArg<int32_t>(args);
                if(numberOfCharacters < 0)
                    throw BuiltinFunctionArgumentError(
                        L"input function argument must be positive", callSource, callPosition
                    );
                std::wstring read;
                read.resize(static_cast<size_t>(numberOfCharacters));
                input.read(&read[0], numberOfCharacters);
                if(input.bad())
                    throw StandardInputError(L"Standard input stream returned error", callSource, callPosition);
                read.resize(input.gcount());
                return Object{{STR}, read};
            }
        )
    };
}

const BuiltinFunction builtinLen = {
    FunctionIdentification(L"len", {{STR}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {STR}, L"string", false)}, {{INT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            std::wstring string = std::get<std::wstring>(args[0].get().value);
            return Object{{INT}, static_cast<int32_t>(string.size())};
        }
    )
};

const BuiltinFunction builtinAbsFloat = {
    FunctionIdentification(L"abs", {{FLOAT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {FLOAT}, L"value", false)}, {{FLOAT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto value = getArg<double>(args);
            return Object{{FLOAT}, std::abs(value)};
        }
    )
};

const BuiltinFunction builtinAbsInt = {
    FunctionIdentification(L"abs", {{INT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>", {VariableDeclaration({0, 0}, {INT}, L"value", false)}, {{INT}},
        [](Position callPosition, const std::wstring &callSource,
           std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto value = getArg<int32_t>(args);
            if(value < -std::numeric_limits<int32_t>::max())
                throw IntegerRangeError(
                    std::format(L"Taking absolute value of the integer {} would exceed integer value range", value),
                    callSource, callPosition
                );
            return Object{{INT}, std::abs(value)};
        }
    )
};

template <typename T>
std::pair<T, T> getTwoArgs(const std::vector<std::reference_wrapper<Object>> &args)
{
    T first = std::get<T>(args[0].get().value);
    T second = std::get<T>(args[1].get().value);
    return {first, second};
}

const BuiltinFunction builtinMaxFloat = {
    FunctionIdentification(L"max", {{FLOAT}, {FLOAT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>",
        {VariableDeclaration({0, 0}, {FLOAT}, L"first", false), VariableDeclaration({0, 0}, {FLOAT}, L"second", false)},
        {{FLOAT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto [first, second] = getTwoArgs<double>(args);
            return Object{{FLOAT}, std::max(first, second)};
        }
    )
};

const BuiltinFunction builtinMaxInt = {
    FunctionIdentification(L"max", {{INT}, {INT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>",
        {VariableDeclaration({0, 0}, {INT}, L"first", false), VariableDeclaration({0, 0}, {INT}, L"second", false)},
        {{INT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto [first, second] = getTwoArgs<int32_t>(args);
            return Object{{INT}, std::max(first, second)};
        }
    )
};

const BuiltinFunction builtinMinFloat = {
    FunctionIdentification(L"min", {{FLOAT}, {FLOAT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>",
        {VariableDeclaration({0, 0}, {FLOAT}, L"first", false), VariableDeclaration({0, 0}, {FLOAT}, L"second", false)},
        {{FLOAT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto [first, second] = getTwoArgs<double>(args);
            return Object{{FLOAT}, std::min(first, second)};
        }
    )
};

const BuiltinFunction builtinMinInt = {
    FunctionIdentification(L"min", {{INT}, {INT}}),
    BuiltinFunctionDeclaration(
        {0, 0}, L"<builtins>",
        {VariableDeclaration({0, 0}, {INT}, L"first", false), VariableDeclaration({0, 0}, {INT}, L"second", false)},
        {{INT}},
        [](Position, const std::wstring &, std::vector<std::reference_wrapper<Object>> args) -> std::optional<Object> {
            auto [first, second] = getTwoArgs<int32_t>(args);
            return Object{{INT}, std::min(first, second)};
        }
    )
};

Program prepareBuiltinFunctions(
    Position programPosition, const std::vector<std::wstring> &arguments, std::wistream &input, std::wostream &output
)
{
    Program program(programPosition);
    program.add(builtinNoArguments(arguments));
    program.add(builtinArgument(arguments));
    program.add(builtinPrint(output));
    program.add(builtinPrintln(output));
    program.add(builtinInputLine(input));
    program.add(builtinInput(input));
    program.add(builtinLen);
    program.add(builtinAbsFloat);
    program.add(builtinAbsInt);
    program.add(builtinMaxFloat);
    program.add(builtinMaxInt);
    program.add(builtinMinFloat);
    program.add(builtinMinInt);
    return program;
}
