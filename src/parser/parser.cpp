#include "parser.hpp"

#include "parserExceptions.hpp"

#include <unordered_map>

using enum TokenType;

Parser::Parser(ILexer &source): source(source), current(source.getNextToken()), next(source.getNextToken()) {}

void Parser::advance()
{
    this->current = this->next;
    this->next = source.getNextToken();
}

// TOP_STMT = INCLUDE_STMT
//          | STRUCT_DECL
//          | VARIANT_DECL
//          | FUNCTION_DECL ;
bool Parser::tryAddTopLevelStatement(Program &program)
{
    std::optional<IncludeStatement> includeBuilt;
    if((includeBuilt = parseIncludeStatement()))
    {
        program.includes.push_back(*includeBuilt);
        return true;
    }
    std::optional<std::pair<std::wstring, StructDeclaration>> structBuilt;
    if((structBuilt = parseStructDeclaration()))
    {
        program.structs.insert(std::move(*structBuilt));
        return true;
    }
    std::optional<std::pair<std::wstring, VariantDeclaration>> variantBuilt;
    if((variantBuilt = parseVariantDeclaration()))
    {
        program.variants.insert(std::move(*variantBuilt));
        return true;
    }
    std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> functionBuilt;
    if((functionBuilt = parseFunctionDeclaration()))
    {
        program.functions.insert(std::move(*functionBuilt));
        return true;
    }
    return false;
}

// INCLUDE_STMT = 'include', STRING_LITERAL ;
std::optional<IncludeStatement> Parser::parseIncludeStatement()
{
    if(current.getType() != KW_INCLUDE)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();
    if(current.getType() != STR_LITERAL)
        throw SyntaxError(std::format(L"Expected a string literal, got {}", current), current.getPosition());

    IncludeStatement built(begin, std::get<std::wstring>(current.getValue()));
    advance();
    return built;
}

// STRUCT_DECL = 'struct', DECL_BLOCK ;
std::optional<std::pair<std::wstring, StructDeclaration>> Parser::parseStructDeclaration()
{
    if(current.getType() != KW_STRUCT)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();
    auto [structName, fields] = parseDeclarationBlock();
    return std::pair{structName, StructDeclaration(begin, fields)};
}

// VARIANT_DECL = 'variant', DECL_BLOCK ;
std::optional<std::pair<std::wstring, VariantDeclaration>> Parser::parseVariantDeclaration()
{
    if(current.getType() != KW_VARIANT)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();
    auto [variantName, fields] = parseDeclarationBlock();
    return std::pair{variantName, VariantDeclaration(begin, fields)};
}

// DECL_BLOCK = IDENTIFIER, '{', FIELD_DECL, { FIELD_DECL } , '}' ;
std::pair<std::wstring, std::vector<Field>> Parser::parseDeclarationBlock()
{
    if(current.getType() != IDENTIFIER)
        throw SyntaxError(std::format(L"Expected an identifier, got {}", current), current.getPosition());
    std::wstring name = std::get<std::wstring>(current.getValue());
    advance();

    if(current.getType() != LBRACE)
        throw SyntaxError(std::format(L"Expected {{, got {}", current), current.getPosition());

    std::vector<Field> fields;
    std::optional<Field> fieldBuilt = parseField();
    if(!fieldBuilt.has_value())
        throw SyntaxError(std::format(L"Expected a type identifier, got {}", current), current.getPosition());
    do
    {
        fields.push_back(*fieldBuilt);
    }
    while((fieldBuilt = parseField()));

    if(current.getType() != RBRACE)
        throw SyntaxError(std::format(L"Expected }}, got {}", current), current.getPosition());
    advance();
    return {name, fields};
}

// FIELD_DECL = TYPE_IDENT, IDENTIFIER, ';' ;
std::optional<Field> Parser::parseField()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        return std::nullopt;

    if(current.getType() != IDENTIFIER)
        throw SyntaxError(std::format(L"Expected an identifier, got {}", current), current.getPosition());
    std::wstring name = std::get<std::wstring>(current.getValue());
    advance();

    if(current.getType() != SEMICOLON)
        throw SyntaxError(std::format(L"Expected ;, got {}", current), current.getPosition());
    advance();
    return Field(begin, *type, name);
}

// TYPE_IDENT = BUILTIN_TYPE
//            | IDENTIFIER ;
std::optional<std::wstring> Parser::parseTypeIdentifier()
{
    std::optional<std::wstring> builtinType;
    if((builtinType = parseBuiltinType()))
        return *builtinType;
    if(current.getType() != IDENTIFIER)
        return std::nullopt;
    std::wstring type = std::get<std::wstring>(current.getValue());
    advance();
    return type;
}

// BUILTIN_TYPE = 'int'
//              | 'float'
//              | 'str'
//              | 'bool' ;
std::optional<std::wstring> Parser::parseBuiltinType()
{
    if(current.getType() != KW_INT && current.getType() != KW_FLOAT && current.getType() != KW_STR &&
       current.getType() != KW_BOOL)
        return std::nullopt;
    std::wstring typeAsString = std::format(L"{}", current);
    advance();
    return typeAsString;
}

// PROGRAM = { TOP_STMT } ;
Program Parser::parseProgram()
{
    Program program(current.getPosition());
    while(tryAddTopLevelStatement(program))
        ;
    return program;
}
