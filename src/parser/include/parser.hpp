#ifndef PARSER_HPP
#define PARSER_HPP

#include "documentTree.hpp"
#include "iLexer.hpp"
#include "parserExceptions.hpp"

class Parser
{
public:
    Parser(ILexer &source);
    Program parseProgram();
private:
    ILexer &source;
    Token current, next;
    void advance();
    void checkAndAdvance(TokenType type);

    template <typename TypeToLoad>
    TypeToLoad loadAndAdvance(TokenType type)
    {
        if(current.getType() != type)
            throw SyntaxError(std::format(L"Expected {}, got {}", type, current), current.getPosition());
        TypeToLoad loaded = std::get<TypeToLoad>(current.getValue());
        advance();
        return loaded;
    }

    bool tryAddTopLevelStatement(Program &program);
    std::optional<IncludeStatement> parseIncludeStatement();
    std::optional<std::pair<std::wstring, StructDeclaration>> parseStructDeclaration();
    std::optional<std::pair<std::wstring, VariantDeclaration>> parseVariantDeclaration();
    // std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> parseFunctionDeclaration();
    std::pair<std::wstring, std::vector<Field>> parseDeclarationBlock();
    std::optional<Field> parseField();
    std::optional<std::wstring> parseTypeIdentifier();
    std::optional<std::wstring> parseBuiltinType();
};

#endif
