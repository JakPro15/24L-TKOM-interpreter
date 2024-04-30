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

std::wstring Parser::loadAndAdvance(TokenType type)
{
    if(current.getType() != type)
        throw SyntaxError(std::format(L"Expected {}, got {}", type, current), current.getPosition());
    std::wstring loaded = std::get<std::wstring>(current.getValue());
    advance();
    return loaded;
}

// PROGRAM = { TOP_STMT } ;
Program Parser::parseProgram()
{
    Program program(current.getPosition());
    while(tryAddTopLevelStatement(program))
        ;
    return program;
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

    std::wstring filePath = loadAndAdvance(STR_LITERAL);
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
    std::wstring name = loadAndAdvance(IDENTIFIER);
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

    std::wstring name = loadAndAdvance(IDENTIFIER);
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
    Position begin = current.getPosition();
    advance();

    std::wstring name = loadAndAdvance(IDENTIFIER);
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
    std::vector<std::unique_ptr<Instruction>> body = parseInstructionBlock();
    std::vector<std::wstring> parameterTypes;
    for(VariableDeclaration parameter: parameters)
        parameterTypes.push_back(parameter.type);

    return std::pair{
        FunctionIdentification(name, parameterTypes),
        FunctionDeclaration(begin, parameters, returnType, std::move(body))
    };
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
    return parameters;
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
    std::wstring name = loadAndAdvance(IDENTIFIER);
    return {isMutable, name};
}

// INSTR_BLOCK = '{', { INSTRUCTION } , '}' ;
std::vector<std::unique_ptr<Instruction>> Parser::parseInstructionBlock()
{
    checkAndAdvance(LBRACE);
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::unique_ptr<Instruction> instruction;
    while((instruction = parseInstruction()))
        instructions.push_back(std::move(instruction));
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

std::unique_ptr<Instruction> Parser::parseDeclOrAssignOrFunCall()
{
    if(current.getType() != IDENTIFIER)
        return std::unique_ptr<Instruction>(nullptr);
    Token firstIdentifier = current;
    advance();

    std::unique_ptr<Instruction> instruction;
    if(!(instruction = parseVariableDeclStatement(firstIdentifier)) &&
       !(instruction = parseAssignmentStatement(firstIdentifier)) &&
       !(instruction = parseFunctionCall(firstIdentifier)))
        throw SyntaxError(
            std::format(L"Expected a variable declaration, an assignment or a function call, got {}", current),
            current.getPosition()
        );
    checkAndAdvance(SEMICOLON);
    return instruction;
}

// IDENTIFIER, VAR_DECL_BODY, '=', EXPRESSION
std::unique_ptr<VariableDeclStatement> Parser::parseVariableDeclStatement(Token firstToken)
{
    if(current.getType() != DOLLAR_SIGN && current.getType() != IDENTIFIER)
        return std::unique_ptr<VariableDeclStatement>(nullptr);

    Position begin = firstToken.getPosition();
    std::wstring type = std::get<std::wstring>(firstToken.getValue());
    auto [isMutable, name] = parseVariableDeclarationBody();
    checkAndAdvance(OP_ASSIGN);
    std::unique_ptr<Expression> value;
    if(!(value = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    return std::make_unique<VariableDeclStatement>(
        begin, VariableDeclaration(begin, type, name, isMutable), std::move(value)
    );
}

// IDENTIFIER, { '.', IDENTIFIER }, '=', EXPRESSION
std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement(Token firstToken)
{
    if(current.getType() != OP_DOT || current.getType() != OP_ASSIGN)
        return std::unique_ptr<AssignmentStatement>(nullptr);

    Position begin = firstToken.getPosition();
    Assignable leftAssignable(begin, std::get<std::wstring>(firstToken.getValue()));
    while(current.getType() == OP_DOT)
    {
        advance();
        std::wstring right = loadAndAdvance(IDENTIFIER);
        leftAssignable = Assignable(begin, std::make_unique<Assignable>(std::move(leftAssignable)), right);
    }
    std::unique_ptr<Expression> value;
    if(!(value = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    return std::make_unique<AssignmentStatement>(firstToken.getPosition(), std::move(leftAssignable), std::move(value));
}

// IDENTIFIER, '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')'
std::unique_ptr<FunctionCall> Parser::parseFunctionCall(Token firstToken)
{
    Position begin = firstToken.getPosition();
    std::wstring name = std::get<std::wstring>(firstToken.getValue());
    checkAndAdvance(LPAREN);

    std::vector<std::unique_ptr<Expression>> parameters;
    std::unique_ptr<Expression> parameterBuilt;
    if(!(parameterBuilt = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    parameters.push_back(std::move(parameterBuilt));
    while(current.getType() == COMMA)
    {
        advance();
        if(!(parameterBuilt = parseExpression()))
            throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
        parameters.push_back(std::move(parameterBuilt));
    }
    checkAndAdvance(RPAREN);
    return std::make_unique<FunctionCall>(begin, name, std::move(parameters));
}

std::unique_ptr<FunctionCall> Parser::parseFunctionCall()
{
    if(current.getType() != IDENTIFIER)
        return std::unique_ptr<FunctionCall>(nullptr);
    Token firstToken = current;
    advance();
    return parseFunctionCall(firstToken);
}

std::unique_ptr<Expression> Parser::parseExpression()
{
    return parseFunctionCall();
}
