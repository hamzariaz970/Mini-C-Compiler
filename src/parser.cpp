#include "parser.hpp"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace {
std::string binaryOpText(TokenType type) {
    switch (type) {
        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::Percent: return "%";
        case TokenType::Equal: return "==";
        case TokenType::NotEqual: return "!=";
        case TokenType::Less: return "<";
        case TokenType::LessEqual: return "<=";
        case TokenType::Greater: return ">";
        case TokenType::GreaterEqual: return ">=";
        case TokenType::And: return "&&";
        case TokenType::Or: return "||";
        default: return "?";
    }
}

std::string numberToText(double value, const Type& type) {
    std::ostringstream out;
    if (type.base == BaseType::Int || type.base == BaseType::Char || type.base == BaseType::Bool) {
        out << static_cast<long long>(value);
    } else {
        out << std::setprecision(10) << value;
    }
    return out.str();
}
}

Parser::Parser(std::vector<Token> tokens, Diagnostics& diagnostics, ParserOptions options)
    : tokens(std::move(tokens)), diagnostics(diagnostics), options(options) {}

bool Parser::parseProgram() {
    try {
        while (!isAtEnd()) {
            astRoot->addChild(parseExternalDeclaration());
        }

        const Symbol* mainFunction = symbolTable.resolve("main");
        if (!mainFunction || mainFunction->kind != SymbolKind::Function || !mainFunction->defined) {
            diagnostics.error(DiagnosticPhase::Semantic, 1, 1, "program must define int main() or void main()");
        } else if (mainFunction->returnType.base != BaseType::Int &&
                   mainFunction->returnType.base != BaseType::Void) {
            diagnostics.error(DiagnosticPhase::Semantic, 1, 1, "main must return int or void");
        } else if (!mainFunction->parameters.empty()) {
            diagnostics.error(DiagnosticPhase::Semantic, 1, 1, "main must not declare parameters in this Mini C subset");
        }
        if (!diagnostics.hasErrors() && options.optimize) {
            tacProgram.eliminateDeadCode();
        }
    } catch (const ParseFailure&) {
        synchronize();
    }
    return !diagnostics.hasErrors();
}

const SymbolTable& Parser::symbols() const {
    return symbolTable;
}

const TACProgram& Parser::tac() const {
    return tacProgram;
}

const ASTNodePtr& Parser::ast() const {
    return astRoot;
}

const Token& Parser::peek() const {
    return tokens[pos];
}

const Token& Parser::previous() const {
    return tokens[pos - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::End;
}

bool Parser::check(TokenType type) const {
    return !isAtEnd() && peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

Token Parser::advance() {
    if (!isAtEnd()) ++pos;
    return previous();
}

Token Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    reportHere(message + ", found '" + peek().lexeme + "'");
    throw ParseFailure(message);
}

void Parser::reportHere(const std::string& message) {
    diagnostics.error(DiagnosticPhase::Syntax, peek().line, peek().column, message);
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        switch (peek().type) {
            case TokenType::KwInt:
            case TokenType::KwFloat:
            case TokenType::KwChar:
            case TokenType::KwBool:
            case TokenType::KwVoid:
            case TokenType::KwIf:
            case TokenType::KwWhile:
            case TokenType::KwFor:
            case TokenType::KwReturn:
                return;
            default:
                advance();
                break;
        }
    }
}

bool Parser::isTypeSpecifier(TokenType type) const {
    return type == TokenType::KwInt || type == TokenType::KwFloat ||
           type == TokenType::KwChar || type == TokenType::KwBool ||
           type == TokenType::KwVoid;
}

Type Parser::parseTypeSpecifier() {
    Token token = advance();
    switch (token.type) {
        case TokenType::KwInt: return Type::scalar(BaseType::Int);
        case TokenType::KwFloat: return Type::scalar(BaseType::Float);
        case TokenType::KwChar: return Type::scalar(BaseType::Char);
        case TokenType::KwBool: return Type::scalar(BaseType::Bool);
        case TokenType::KwVoid: return Type::scalar(BaseType::Void);
        default:
            diagnostics.error(DiagnosticPhase::Syntax, token.line, token.column, "expected type specifier");
            return Type::scalar(BaseType::Error);
    }
}

ASTNodePtr Parser::parseExternalDeclaration() {
    if (!isTypeSpecifier(peek().type)) {
        reportHere("expected top-level declaration");
        throw ParseFailure("expected top-level declaration");
    }

    Type baseType = parseTypeSpecifier();
    Token name = expect(TokenType::Identifier, "expected identifier after type");

    if (check(TokenType::LeftParen)) {
        return parseFunction(baseType, name);
    }
    return parseVariableDeclaration(baseType, name, true);
}

ASTNodePtr Parser::parseFunction(Type returnType, const Token& name) {
    expect(TokenType::LeftParen, "expected '(' after function name");
    std::vector<Symbol> params = parseParameters();
    expect(TokenType::RightParen, "expected ')' after parameter list");

    ASTNodePtr functionNode = makeNode("FunctionDeclaration", name.lexeme);
    functionNode->addChild(makeNode("ReturnType", returnType.str()));
    ASTNodePtr paramsNode = makeNode("Parameters");
    for (const Symbol& param : params) {
        ASTNodePtr paramNode = makeNode("Parameter", param.name);
        paramNode->addChild(makeNode("Type", param.type.str()));
        paramsNode->addChild(paramNode);
    }
    functionNode->addChild(paramsNode);

    Symbol function;
    function.name = name.lexeme;
    function.kind = SymbolKind::Function;
    function.returnType = returnType;
    function.type = returnType;
    for (const Symbol& param : params) function.parameters.push_back(param.type);

    if (match(TokenType::Semicolon)) {
        function.defined = false;
        symbolTable.declare(function, name.line, name.column, diagnostics);
        return functionNode;
    }

    function.defined = true;
    symbolTable.declare(function, name.line, name.column, diagnostics);

    currentFunction = name.lexeme;
    currentReturnType = returnType;
    currentFunctionHasReturn = false;
    tacProgram.emit("func " + name.lexeme);

    symbolTable.enterScope();
    for (const Symbol& param : params) {
        symbolTable.declare(param, name.line, name.column, diagnostics);
        tacProgram.emit("param_in " + param.name);
    }
    functionNode->addChild(parseBlock(false));
    symbolTable.exitScope();

    if (returnType.base != BaseType::Void && !currentFunctionHasReturn) {
        diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column,
                          "non-void function '" + name.lexeme + "' must return a value");
    }

    tacProgram.emit("endfunc " + name.lexeme);
    currentFunction.clear();
    currentReturnType = Type::scalar(BaseType::Void);
    currentFunctionHasReturn = false;
    return functionNode;
}

std::vector<Symbol> Parser::parseParameters() {
    std::vector<Symbol> params;
    if (check(TokenType::RightParen)) return params;

    do {
        Type type = parseTypeSpecifier();
        if (type.base == BaseType::Void && !check(TokenType::Identifier)) {
            return params;
        }
        Token name = expect(TokenType::Identifier, "expected parameter name");
        Type finalType = parseDeclaratorType(type);
        if (finalType.base == BaseType::Void) {
            diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column, "parameter '" + name.lexeme + "' cannot have type void");
        }

        Symbol param;
        param.name = name.lexeme;
        param.kind = SymbolKind::Parameter;
        param.type = finalType;
        params.push_back(param);
    } while (match(TokenType::Comma));

    return params;
}

ASTNodePtr Parser::parseVariableDeclaration(Type baseType, Token firstName, bool global) {
    ASTNodePtr declarationNode = makeNode(global ? "GlobalDeclaration" : "VariableDeclaration");
    Token name = firstName;
    while (true) {
        Type finalType = parseDeclaratorType(baseType);
        if (finalType.base == BaseType::Void) {
            diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column, "variable '" + name.lexeme + "' cannot have type void");
        }

        Symbol symbol;
        symbol.name = name.lexeme;
        symbol.kind = SymbolKind::Variable;
        symbol.type = finalType;
        bool declared = symbolTable.declare(symbol, name.line, name.column, diagnostics);
        if (declared) {
            tacProgram.emit(std::string(global ? "global " : "declare ") + finalType.str() + " " + name.lexeme);
        }

        ASTNodePtr variableNode = makeNode("Declarator", name.lexeme);
        variableNode->addChild(makeNode("Type", finalType.str()));

        if (match(TokenType::Assign)) {
            Token assignToken = previous();
            ExprResult right = parseExpression();
            if (declared) {
                ExprResult left;
                left.type = finalType;
                left.place = name.lexeme;
                left.target = name.lexeme;
                left.isLValue = true;
                emitAssignment(left, right, assignToken);
            }
            ASTNodePtr initializerNode = makeNode("Initializer");
            initializerNode->addChild(right.ast);
            variableNode->addChild(initializerNode);
        }

        declarationNode->addChild(variableNode);

        if (!match(TokenType::Comma)) break;
        name = expect(TokenType::Identifier, "expected variable name after ','");
    }

    expect(TokenType::Semicolon, "expected ';' after declaration");
    return declarationNode;
}

Type Parser::parseDeclaratorType(Type baseType) {
    if (!match(TokenType::LeftBracket)) return baseType;

    Token size = expect(TokenType::IntLiteral, "expected integer array size");
    expect(TokenType::RightBracket, "expected ']' after array size");
    int arraySize = std::atoi(size.lexeme.c_str());
    if (arraySize <= 0) {
        diagnostics.error(DiagnosticPhase::Semantic, size.line, size.column, "array size must be greater than zero");
    }
    return Type::array(baseType.base, arraySize);
}

ASTNodePtr Parser::parseBlock(bool createScope) {
    ASTNodePtr blockNode = makeNode("Block");
    expect(TokenType::LeftBrace, "expected '{' to start block");
    if (createScope) symbolTable.enterScope();

    while (!isAtEnd() && !check(TokenType::RightBrace)) {
        try {
            blockNode->addChild(parseStatement());
        } catch (const ParseFailure&) {
            synchronize();
            if (!isAtEnd() && !check(TokenType::RightBrace)) advance();
        }
    }

    expect(TokenType::RightBrace, "expected '}' after block");
    if (createScope) symbolTable.exitScope();
    return blockNode;
}

ASTNodePtr Parser::parseStatement() {
    if (check(TokenType::LeftBrace)) {
        return parseBlock(true);
    } else if (match(TokenType::KwIf)) {
        return parseIf();
    } else if (match(TokenType::KwWhile)) {
        return parseWhile();
    } else if (match(TokenType::KwFor)) {
        return parseFor();
    } else if (match(TokenType::KwReturn)) {
        return parseReturn();
    } else if (isTypeSpecifier(peek().type)) {
        return parseLocalDeclaration();
    } else if (match(TokenType::Semicolon)) {
        return makeNode("EmptyStatement");
    } else {
        ExprResult expr = parseExpression();
        expect(TokenType::Semicolon, "expected ';' after expression");
        ASTNodePtr expressionNode = makeNode("ExpressionStatement");
        expressionNode->addChild(expr.ast);
        return expressionNode;
    }
}

ASTNodePtr Parser::parseIf() {
    expect(TokenType::LeftParen, "expected '(' after if");
    ExprResult condition = ensureValue(parseExpression());
    expect(TokenType::RightParen, "expected ')' after if condition");
    if (!canUseAsCondition(condition.type)) {
        diagnostics.error(DiagnosticPhase::Semantic, previous().line, previous().column, "if condition must be numeric or bool");
    }

    std::string elseLabel = tacProgram.newLabel();
    std::string endLabel = tacProgram.newLabel();
    tacProgram.emit("ifFalse " + condition.place + " goto " + elseLabel);
    ASTNodePtr ifNode = makeNode("IfStatement");
    ASTNodePtr conditionNode = makeNode("Condition");
    conditionNode->addChild(condition.ast);
    ifNode->addChild(conditionNode);
    ASTNodePtr thenNode = makeNode("Then");
    thenNode->addChild(parseStatement());
    ifNode->addChild(thenNode);
    tacProgram.emit("goto " + endLabel);
    tacProgram.emitLabel(elseLabel);
    if (match(TokenType::KwElse)) {
        ASTNodePtr elseNode = makeNode("Else");
        elseNode->addChild(parseStatement());
        ifNode->addChild(elseNode);
    }
    tacProgram.emitLabel(endLabel);
    return ifNode;
}

ASTNodePtr Parser::parseWhile() {
    std::string startLabel = tacProgram.newLabel();
    std::string endLabel = tacProgram.newLabel();
    tacProgram.emitLabel(startLabel);

    expect(TokenType::LeftParen, "expected '(' after while");
    ExprResult condition = ensureValue(parseExpression());
    expect(TokenType::RightParen, "expected ')' after while condition");
    if (!canUseAsCondition(condition.type)) {
        diagnostics.error(DiagnosticPhase::Semantic, previous().line, previous().column, "while condition must be numeric or bool");
    }

    tacProgram.emit("ifFalse " + condition.place + " goto " + endLabel);
    ASTNodePtr whileNode = makeNode("WhileStatement");
    ASTNodePtr conditionNode = makeNode("Condition");
    conditionNode->addChild(condition.ast);
    whileNode->addChild(conditionNode);
    ASTNodePtr bodyNode = makeNode("Body");
    bodyNode->addChild(parseStatement());
    whileNode->addChild(bodyNode);
    tacProgram.emit("goto " + startLabel);
    tacProgram.emitLabel(endLabel);
    return whileNode;
}

ASTNodePtr Parser::parseFor() {
    ASTNodePtr forNode = makeNode("ForStatement");
    symbolTable.enterScope();
    expect(TokenType::LeftParen, "expected '(' after for");

    if (isTypeSpecifier(peek().type)) {
        ASTNodePtr initializerNode = makeNode("Initializer");
        initializerNode->addChild(parseLocalDeclaration());
        forNode->addChild(initializerNode);
    } else if (!match(TokenType::Semicolon)) {
        ExprResult initializer = parseExpression();
        expect(TokenType::Semicolon, "expected ';' after for initializer");
        ASTNodePtr initializerNode = makeNode("Initializer");
        initializerNode->addChild(initializer.ast);
        forNode->addChild(initializerNode);
    } else {
        forNode->addChild(makeNode("Initializer"));
    }

    std::string startLabel = tacProgram.newLabel();
    std::string endLabel = tacProgram.newLabel();
    tacProgram.emitLabel(startLabel);

    if (!check(TokenType::Semicolon)) {
        ExprResult condition = ensureValue(parseExpression());
        if (!canUseAsCondition(condition.type)) {
            diagnostics.error(DiagnosticPhase::Semantic, previous().line, previous().column, "for condition must be numeric or bool");
        }
        tacProgram.emit("ifFalse " + condition.place + " goto " + endLabel);
        ASTNodePtr conditionNode = makeNode("Condition");
        conditionNode->addChild(condition.ast);
        forNode->addChild(conditionNode);
    } else {
        forNode->addChild(makeNode("Condition"));
    }
    expect(TokenType::Semicolon, "expected ';' after for condition");

    std::size_t updateStart = tacProgram.mark();
    ASTNodePtr updateNode = makeNode("Update");
    if (!check(TokenType::RightParen)) {
        ExprResult update = parseExpression();
        updateNode->addChild(update.ast);
    }
    std::vector<std::string> updateCode = tacProgram.takeFrom(updateStart);
    forNode->addChild(updateNode);
    expect(TokenType::RightParen, "expected ')' after for clauses");

    ASTNodePtr bodyNode = makeNode("Body");
    bodyNode->addChild(parseStatement());
    forNode->addChild(bodyNode);
    tacProgram.append(updateCode);
    tacProgram.emit("goto " + startLabel);
    tacProgram.emitLabel(endLabel);
    symbolTable.exitScope();
    return forNode;
}

ASTNodePtr Parser::parseReturn() {
    Token returnToken = previous();
    ASTNodePtr returnNode = makeNode("ReturnStatement");
    currentFunctionHasReturn = true;
    if (currentFunction.empty()) {
        diagnostics.error(DiagnosticPhase::Semantic, returnToken.line, returnToken.column, "return statement outside function");
    }

    if (match(TokenType::Semicolon)) {
        if (currentReturnType.base != BaseType::Void) {
            diagnostics.error(DiagnosticPhase::Semantic, returnToken.line, returnToken.column,
                              "non-void function '" + currentFunction + "' must return a value");
        }
        tacProgram.emit("return");
        return returnNode;
    }

    ExprResult value = ensureValue(parseExpression());
    expect(TokenType::Semicolon, "expected ';' after return value");

    if (currentReturnType.base == BaseType::Void) {
        diagnostics.error(DiagnosticPhase::Semantic, returnToken.line, returnToken.column,
                          "void function '" + currentFunction + "' must not return a value");
    } else if (!canAssign(value.type, currentReturnType)) {
        diagnostics.error(DiagnosticPhase::Semantic, returnToken.line, returnToken.column,
                          "cannot return " + value.type.str() + " from function returning " +
                              currentReturnType.str());
    }
    tacProgram.emit("return " + value.place);
    returnNode->addChild(value.ast);
    return returnNode;
}

ASTNodePtr Parser::parseLocalDeclaration() {
    Type type = parseTypeSpecifier();
    Token name = expect(TokenType::Identifier, "expected variable name");
    return parseVariableDeclaration(type, name, false);
}

ExprResult Parser::parseExpression() {
    return parseAssignment();
}

ExprResult Parser::parseAssignment() {
    ExprResult left = parseLogicalOr();
    if (match(TokenType::Assign)) {
        Token op = previous();
        ExprResult right = parseAssignment();
        emitAssignment(left, right, op);
        ASTNodePtr assignmentNode = makeNode("AssignmentExpression", op.lexeme);
        assignmentNode->addChild(left.ast);
        assignmentNode->addChild(right.ast);
        left.ast = assignmentNode;
        left.isConstant = false;
        return left;
    }
    return left;
}

ExprResult Parser::parseLogicalOr() {
    ExprResult left = parseLogicalAnd();
    while (match(TokenType::Or)) {
        Token op = previous();
        ExprResult right = parseLogicalAnd();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseLogicalAnd() {
    ExprResult left = parseEquality();
    while (match(TokenType::And)) {
        Token op = previous();
        ExprResult right = parseEquality();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseEquality() {
    ExprResult left = parseRelational();
    while (match(TokenType::Equal) || match(TokenType::NotEqual)) {
        Token op = previous();
        ExprResult right = parseRelational();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseRelational() {
    ExprResult left = parseAdditive();
    while (match(TokenType::Less) || match(TokenType::LessEqual) ||
           match(TokenType::Greater) || match(TokenType::GreaterEqual)) {
        Token op = previous();
        ExprResult right = parseAdditive();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseAdditive() {
    ExprResult left = parseMultiplicative();
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        Token op = previous();
        ExprResult right = parseMultiplicative();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseMultiplicative() {
    ExprResult left = parseUnary();
    while (match(TokenType::Star) || match(TokenType::Slash) || match(TokenType::Percent)) {
        Token op = previous();
        ExprResult right = parseUnary();
        left = emitBinary(left, op, right);
    }
    return left;
}

ExprResult Parser::parseUnary() {
    if (match(TokenType::Not) || match(TokenType::Minus) ||
        match(TokenType::Increment) || match(TokenType::Decrement)) {
        Token op = previous();
        ExprResult operand = parseUnary();
        return emitUnary(op, operand);
    }
    return parsePostfix();
}

ExprResult Parser::parsePostfix() {
    ExprResult expr = parsePrimary();
    while (true) {
        if (match(TokenType::LeftBracket)) {
            Token bracket = previous();
            ExprResult index = ensureValue(parseExpression());
            expect(TokenType::RightBracket, "expected ']' after array index");
            if (!expr.type.isArray) {
                diagnostics.error(DiagnosticPhase::Semantic, bracket.line, bracket.column, "subscripted value is not an array");
                expr.type = Type::scalar(BaseType::Error);
            } else if (index.type.base != BaseType::Int && index.type.base != BaseType::Error) {
                diagnostics.error(DiagnosticPhase::Semantic, bracket.line, bracket.column, "array index must be int");
            }
            Type elementType = Type::scalar(expr.type.base);
            expr.type = elementType;
            expr.target = expr.place + "[" + index.place + "]";
            expr.place = expr.target;
            ASTNodePtr subscriptNode = makeNode("SubscriptExpression");
            subscriptNode->addChild(expr.ast);
            subscriptNode->addChild(index.ast);
            expr.ast = subscriptNode;
            expr.isLValue = true;
            expr.isArrayElement = true;
            expr.isConstant = false;
        } else if (match(TokenType::Increment) || match(TokenType::Decrement)) {
            Token op = previous();
            if (!expr.isLValue) {
                diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "postfix operator requires assignable expression");
                return expr;
            }
            ExprResult value = ensureValue(expr);
            std::string temp = tacProgram.newTemp();
            tacProgram.emit(temp + " = " + value.place);
            std::string next = tacProgram.newTemp();
            std::string opText = op.type == TokenType::Increment ? "+" : "-";
            tacProgram.emit(next + " = " + value.place + " " + opText + " 1");
            tacProgram.emit(expr.target + " = " + next);
            ASTNodePtr postfixNode = makeNode("PostfixExpression", op.lexeme);
            postfixNode->addChild(expr.ast);
            expr.ast = postfixNode;
            expr.place = temp;
            expr.isLValue = false;
        } else {
            break;
        }
    }
    return expr;
}

ExprResult Parser::parsePrimary() {
    if (match(TokenType::IntLiteral)) {
        return makeLiteral(Type::scalar(BaseType::Int), previous().lexeme, true,
                           std::strtod(previous().lexeme.c_str(), nullptr));
    }
    if (match(TokenType::FloatLiteral)) {
        return makeLiteral(Type::scalar(BaseType::Float), previous().lexeme, true,
                           std::strtod(previous().lexeme.c_str(), nullptr));
    }
    if (match(TokenType::CharLiteral)) {
        return makeLiteral(Type::scalar(BaseType::Char), previous().lexeme, false, 0);
    }
    if (match(TokenType::StringLiteral)) {
        return makeLiteral(Type::scalar(BaseType::String), previous().lexeme, false, 0);
    }
    if (match(TokenType::KwTrue)) {
        return makeLiteral(Type::scalar(BaseType::Bool), "1", true, 1);
    }
    if (match(TokenType::KwFalse)) {
        return makeLiteral(Type::scalar(BaseType::Bool), "0", true, 0);
    }
    if (match(TokenType::LeftParen)) {
        ExprResult expr = parseExpression();
        expect(TokenType::RightParen, "expected ')' after expression");
        return expr;
    }
    if (match(TokenType::Identifier)) {
        Token name = previous();
        Symbol* symbol = symbolTable.resolve(name.lexeme);
        if (!symbol) {
            diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column, "use of undeclared identifier '" + name.lexeme + "'");
            ExprResult error;
            error.type = Type::scalar(BaseType::Error);
            error.place = name.lexeme;
            error.ast = makeNode("Identifier", name.lexeme);
            return error;
        }

        if (match(TokenType::LeftParen)) {
            std::vector<ExprResult> args;
            if (!check(TokenType::RightParen)) {
                do {
                    args.push_back(ensureValue(parseExpression()));
                } while (match(TokenType::Comma));
            }
            expect(TokenType::RightParen, "expected ')' after arguments");

            if (symbol->kind != SymbolKind::Function) {
                diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column, "'" + name.lexeme + "' is not a function");
            } else {
                checkCallArguments(*symbol, args, name);
            }

            for (const ExprResult& arg : args) tacProgram.emit("param " + arg.place);
            ExprResult call;
            call.type = symbol->kind == SymbolKind::Function ? symbol->returnType : Type::scalar(BaseType::Error);
            call.ast = makeNode("CallExpression", name.lexeme);
            for (const ExprResult& arg : args) call.ast->addChild(arg.ast);
            if (call.type.base == BaseType::Void) {
                tacProgram.emit("call " + name.lexeme + ", " + std::to_string(args.size()));
                call.place = "";
            } else {
                call.place = tacProgram.newTemp();
                tacProgram.emit(call.place + " = call " + name.lexeme + ", " + std::to_string(args.size()));
            }
            return call;
        }

        ExprResult variable;
        variable.type = symbol->type;
        variable.place = name.lexeme;
        variable.target = name.lexeme;
        variable.ast = makeNode("Identifier", name.lexeme);
        variable.isLValue = symbol->kind != SymbolKind::Function;
        return variable;
    }

    reportHere("expected expression");
    throw ParseFailure("expected expression");
}

ExprResult Parser::makeLiteral(const Type& type, const std::string& text, bool numeric, double value) {
    ExprResult result;
    result.type = type;
    result.place = text;
    result.ast = makeNode("Literal", text);
    result.constantText = text;
    result.isConstant = numeric;
    result.numberValue = value;
    return result;
}

ExprResult Parser::ensureValue(const ExprResult& expr) {
    if (expr.isArrayElement) {
        ExprResult value = expr;
        value.place = tacProgram.newTemp();
        tacProgram.emit(value.place + " = " + expr.target);
        value.isLValue = false;
        value.isArrayElement = false;
        ASTNodePtr readNode = makeNode("ArrayRead");
        readNode->addChild(expr.ast);
        value.ast = readNode;
        return value;
    }
    return expr;
}

ExprResult Parser::emitBinary(const ExprResult& rawLeft, const Token& op, const ExprResult& rawRight) {
    ExprResult left = ensureValue(rawLeft);
    ExprResult right = ensureValue(rawRight);
    ExprResult result;

    bool arithmetic = op.type == TokenType::Plus || op.type == TokenType::Minus ||
                      op.type == TokenType::Star || op.type == TokenType::Slash ||
                      op.type == TokenType::Percent;
    bool comparison = op.type == TokenType::Equal || op.type == TokenType::NotEqual ||
                      op.type == TokenType::Less || op.type == TokenType::LessEqual ||
                      op.type == TokenType::Greater || op.type == TokenType::GreaterEqual;
    bool logical = op.type == TokenType::And || op.type == TokenType::Or;

    if (arithmetic) {
        result.type = arithmeticResult(left.type, right.type);
        if (result.type.base == BaseType::Error) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "operator '" + op.lexeme + "' requires numeric operands");
        }
        if (op.type == TokenType::Percent &&
            (left.type.base != BaseType::Int || right.type.base != BaseType::Int)) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "operator '%' requires int operands");
        }
    } else if (comparison) {
        result.type = comparisonResult(left.type, right.type);
        if (result.type.base == BaseType::Error) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "incompatible operands for '" + op.lexeme + "'");
        }
    } else if (logical) {
        if (!canUseAsCondition(left.type) || !canUseAsCondition(right.type)) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "logical operator requires numeric or bool operands");
        }
        result.type = Type::scalar(BaseType::Bool);
    }

    if (options.optimize && left.isConstant && right.isConstant && result.type.base != BaseType::Error) {
        double value = 0.0;
        switch (op.type) {
            case TokenType::Plus: value = left.numberValue + right.numberValue; break;
            case TokenType::Minus: value = left.numberValue - right.numberValue; break;
            case TokenType::Star: value = left.numberValue * right.numberValue; break;
            case TokenType::Slash:
                value = right.numberValue == 0 ? 0 : left.numberValue / right.numberValue;
                break;
            case TokenType::Percent:
                value = static_cast<int>(left.numberValue) % static_cast<int>(right.numberValue);
                break;
            case TokenType::Equal: value = left.numberValue == right.numberValue; break;
            case TokenType::NotEqual: value = left.numberValue != right.numberValue; break;
            case TokenType::Less: value = left.numberValue < right.numberValue; break;
            case TokenType::LessEqual: value = left.numberValue <= right.numberValue; break;
            case TokenType::Greater: value = left.numberValue > right.numberValue; break;
            case TokenType::GreaterEqual: value = left.numberValue >= right.numberValue; break;
            case TokenType::And: value = left.numberValue && right.numberValue; break;
            case TokenType::Or: value = left.numberValue || right.numberValue; break;
            default: break;
        }
        result.isConstant = true;
        result.numberValue = value;
        result.place = numberToText(value, result.type);
        result.ast = makeNode("BinaryExpression", binaryOpText(op.type));
        result.ast->addChild(left.ast);
        result.ast->addChild(right.ast);
        result.constantText = result.place;
        tacProgram.emit("# constant folded " + left.place + " " + binaryOpText(op.type) + " " +
                        right.place + " -> " + result.place);
        return result;
    }

    result.place = tacProgram.newTemp();
    result.ast = makeNode("BinaryExpression", binaryOpText(op.type));
    result.ast->addChild(left.ast);
    result.ast->addChild(right.ast);
    tacProgram.emit(result.place + " = " + left.place + " " + binaryOpText(op.type) + " " + right.place);
    return result;
}

ExprResult Parser::emitUnary(const Token& op, const ExprResult& rawOperand) {
    ExprResult operand = ensureValue(rawOperand);
    ExprResult result;

    if (op.type == TokenType::Not) {
        if (!canUseAsCondition(operand.type)) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "operator '!' requires numeric or bool operand");
        }
        result.type = Type::scalar(BaseType::Bool);
    } else if (op.type == TokenType::Minus) {
        if (!operand.type.isNumeric()) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "unary '-' requires numeric operand");
        }
        result.type = operand.type;
    } else {
        if (!rawOperand.isLValue) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "prefix operator requires assignable expression");
        }
        if (!operand.type.isNumeric()) {
            diagnostics.error(DiagnosticPhase::Semantic, op.line, op.column, "prefix operator requires numeric operand");
        }
        result.type = operand.type;
        std::string opText = op.type == TokenType::Increment ? "+" : "-";
        result.place = tacProgram.newTemp();
        result.ast = makeNode("PrefixExpression", op.lexeme);
        result.ast->addChild(rawOperand.ast);
        tacProgram.emit(result.place + " = " + operand.place + " " + opText + " 1");
        tacProgram.emit(rawOperand.target + " = " + result.place);
        return result;
    }

    if (options.optimize && operand.isConstant) {
        double value = op.type == TokenType::Not ? !operand.numberValue : -operand.numberValue;
        result.isConstant = true;
        result.numberValue = value;
        result.place = numberToText(value, result.type);
        result.ast = makeNode("UnaryExpression", op.lexeme);
        result.ast->addChild(operand.ast);
        tacProgram.emit("# constant folded " + op.lexeme + operand.place + " -> " + result.place);
        return result;
    }

    result.place = tacProgram.newTemp();
    result.ast = makeNode("UnaryExpression", op.lexeme);
    result.ast->addChild(operand.ast);
    tacProgram.emit(result.place + " = " + op.lexeme + operand.place);
    return result;
}

void Parser::emitAssignment(const ExprResult& rawLeft, const ExprResult& rawRight, const Token& opToken) {
    if (!rawLeft.isLValue) {
        diagnostics.error(DiagnosticPhase::Semantic, opToken.line, opToken.column, "left side of assignment is not assignable");
        return;
    }

    ExprResult right = ensureValue(rawRight);
    bool ok = true;
    if (!canAssign(right.type, rawLeft.type)) {
        diagnostics.error(DiagnosticPhase::Semantic, opToken.line, opToken.column,
                          "cannot assign " + right.type.str() + " to " + rawLeft.type.str());
        ok = false;
    }
    if (rawLeft.type.isArray && !rawLeft.isArrayElement) {
        diagnostics.error(DiagnosticPhase::Semantic, opToken.line, opToken.column, "cannot assign to an entire array");
        ok = false;
    }

    if (ok) tacProgram.emit(rawLeft.target + " = " + right.place);
}

bool Parser::checkCallArguments(const Symbol& function, const std::vector<ExprResult>& args, const Token& name) {
    if (args.size() != function.parameters.size()) {
        diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column,
                          "function '" + name.lexeme + "' expects " +
                              std::to_string(function.parameters.size()) + " argument(s), got " +
                              std::to_string(args.size()));
        return false;
    }

    bool ok = true;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (!canAssign(args[i].type, function.parameters[i])) {
            diagnostics.error(DiagnosticPhase::Semantic, name.line, name.column,
                              "argument " + std::to_string(i + 1) + " of '" + name.lexeme +
                                  "' expects " + function.parameters[i].str() + ", got " +
                                  args[i].type.str());
            ok = false;
        }
    }
    return ok;
}
