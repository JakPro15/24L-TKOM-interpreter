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

void Parser::checkAndAdvance(TokenType type)
{
    if(current.getType() != type)
        throw SyntaxError(std::format(L"Expected {}, got {}", type, current), current.getPosition());
    advance();
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

    std::wstring filePath = loadAndAdvance<std::wstring>(STR_LITERAL);
    checkAndAdvance(SEMICOLON);
    return IncludeStatement(begin, filePath);
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
    std::wstring name = loadAndAdvance<std::wstring>(IDENTIFIER);
    checkAndAdvance(LBRACE);

    std::vector<Field> fields;
    std::optional<Field> fieldBuilt = parseField();
    if(!fieldBuilt.has_value())
        throw SyntaxError(std::format(L"Expected a type identifier, got {}", current), current.getPosition());
    do
    {
        fields.push_back(*fieldBuilt);
    }
    while((fieldBuilt = parseField()));

    checkAndAdvance(RBRACE);
    return {name, fields};
}

// FIELD_DECL = TYPE_IDENT, IDENTIFIER, ';' ;
std::optional<Field> Parser::parseField()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        return std::nullopt;

    std::wstring name = loadAndAdvance<std::wstring>(IDENTIFIER);
    checkAndAdvance(SEMICOLON);
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

// FUNCTION_DECL = 'func', IDENTIFIER, '(', [ PARAMETERS ], ')', [ '->', TYPE_IDENT ] , INSTR_BLOCK ;
std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> Parser::parseFunctionDeclaration()
{
    if(current.getType() != KW_FUNC)
        return std::nullopt;

    std::wstring name = loadAndAdvance<std::wstring>(IDENTIFIER);
    checkAndAdvance(LPAREN);
    std::vector<VariableDeclaration> parameters = parseParameters();
    checkAndAdvance(RPAREN);
    std::optional<std::wstring> returnType;
    if(current.getType() == ARROW)
    {
        advance();
        if(!(returnType = parseTypeIdentifier()))
            throw SyntaxError(
                std::format(L"Expected type identifier, got {}", current.getType()), current.getPosition()
            );
    }
}

// PARAMETERS = VARIABLE_DECL, {',', VARIABLE_DECL};
std::vector<VariableDeclaration> Parser::parseParameters()
{
    std::optional<VariableDeclaration> parameter;
    if(!(parameter = parseVariableDeclaration()))
        return {};
    std::vector<VariableDeclaration> parameters{*parameter};
    while(current.getType() == COMMA)
    {
        advance();
        if(!(parameter = parseVariableDeclaration()))
            throw SyntaxError(
                std::format(L"Expected type identifier, got {}", current.getType()), current.getPosition()
            );
        parameters.push_back(*parameter);
    }
}

// VARIABLE_DECL = TYPE_IDENT, VAR_DECL_BODY ;
std::optional<VariableDeclaration> Parser::parseVariableDeclaration()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        return std::nullopt;

    auto [isMutable, name] = parseVariableDeclarationBody();
    return VariableDeclaration(begin, *type, name, isMutable);
}

// VAR_DECL_BODY = IDENTIFIER
//               | '$', IDENTIFIER ;
std::pair<bool, std::wstring> Parser::parseVariableDeclarationBody()
{
    bool isMutable = false;
    if(current.getType() == DOLLAR_SIGN)
    {
        isMutable = true;
        advance();
    }
    std::wstring name = loadAndAdvance<std::wstring>(IDENTIFIER);
    return {isMutable, name};
}

// INSTR_BLOCK = '{', { INSTRUCTION } , '}' ;
std::vector<std::unique_ptr<Instruction>> Parser::parseInstructionBlock()
{
    checkAndAdvance(LBRACE);
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::unique_ptr<Instruction> instruction;
    while((instruction = parseInstruction()))
        instructions.push_back(instruction);
    checkAndAdvance(RBRACE);
    return instructions;
}

// INSTRUCTION =   IDENTIFIER, DECL_OR_ASSIGN_OR_FUNCALL, ';'
//               | BUILTIN_DECL
//               | RETURN_STMT
//               | 'continue', ';'
//               | 'break', ';'
//               | IF_STMT
//               | WHILE_STMT
//               | DO_WHILE_STMT ;
std::unique_ptr<Instruction> Parser::parseInstruction()
{
    Position begin = current.getPosition();
    if(current.getType() == KW_CONTINUE)
    {
        advance();
        checkAndAdvance(SEMICOLON);
        return std::make_unique<ContinueStatement>(begin);
    }
    else if(current.getType() == KW_BREAK)
    {
        advance();
        checkAndAdvance(SEMICOLON);
        return std::make_unique<BreakStatement>(begin);
    }
    return std::unique_ptr<Instruction>(nullptr);
}

// PROGRAM = { TOP_STMT } ;
Program Parser::parseProgram()
{
    Program program(current.getPosition());
    while(tryAddTopLevelStatement(program))
        ;
    return program;
}
