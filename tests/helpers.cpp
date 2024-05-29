#include "helpers.hpp"

std::unique_ptr<Literal> makeLiteral(Position position, std::variant<std::wstring, int32_t, double, bool> value)
{
    return std::make_unique<Literal>(position, value);
}

namespace {
std::vector<VariableDeclaration> createParameters(const FunctionIdentification &id)
{
    std::vector<VariableDeclaration> parameters;
    for(unsigned i = 0; const Type &type: id.parameterTypes)
        parameters.emplace_back(Position{1, 1}, type, std::format(L"a{}", i++), false);
    return parameters;
}
}

std::wstring addOverload(Program &program, const FunctionIdentification &id, std::optional<Type> returnType)
{
    std::vector<std::unique_ptr<Instruction>> instructions;
    if(returnType)
        instructions.push_back(std::make_unique<ReturnStatement>(Position{0, 0}, makeLiteral({0, 0}, 2)));
    auto function = std::make_unique<FunctionDeclaration>(
        Position{1, 1}, L"<test>", createParameters(id), returnType, std::move(instructions)
    );
    std::wstringstream printed;
    std::pair<const FunctionIdentification, std::unique_ptr<BaseFunctionDeclaration>> toPrint{id, std::move(function)};
    PrintingVisitor(printed).visit(toPrint);
    program.functions.insert({id, std::move(toPrint.second)});
    return printed.str();
}

std::wstring wrapInMain(const std::wstring &source)
{
    return L"func main() {\n" + source + L"}";
}
