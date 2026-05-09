#ifndef MINIC_PARSER_HPP
#define MINIC_PARSER_HPP

#include "ast.hpp"
#include "diagnostics.hpp"
#include "symbol_table.hpp"
#include "tac.hpp"
#include "token.hpp"

#include <stdexcept>
#include <string>
#include <vector>

struct ExprResult {
    Type type;
    std::string place;
    std::string target;
    ASTNodePtr ast;
    bool isLValue = false;
    bool isArrayElement = false;
    bool isConstant = false;
    double numberValue = 0.0;
    std::string constantText;
};

struct ParserOptions {
    bool optimize = true;
};

class ParseFailure : public std::runtime_error {
public:
    explicit ParseFailure(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    Parser(std::vector<Token> tokens, Diagnostics& diagnostics, ParserOptions options = {});
    bool parseProgram();
    const SymbolTable& symbols() const;
    const TACProgram& tac() const;
    const ASTNodePtr& ast() const;

private:
    std::vector<Token> tokens;
    Diagnostics& diagnostics;
    ParserOptions options;
    SymbolTable symbolTable;
    TACProgram tacProgram;
    ASTNodePtr astRoot = makeNode("Program");
    std::size_t pos = 0;
    Type currentReturnType = Type::scalar(BaseType::Void);
    std::string currentFunction;
    bool currentFunctionHasReturn = false;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token advance();
    Token expect(TokenType type, const std::string& message);
    void reportHere(const std::string& message);
    void synchronize();

    bool isTypeSpecifier(TokenType type) const;
    Type parseTypeSpecifier();

    ASTNodePtr parseExternalDeclaration();
    ASTNodePtr parseFunction(Type returnType, const Token& name);
    std::vector<Symbol> parseParameters();
    ASTNodePtr parseVariableDeclaration(Type baseType, Token firstName, bool global);
    Type parseDeclaratorType(Type baseType);

    ASTNodePtr parseBlock(bool createScope);
    ASTNodePtr parseStatement();
    ASTNodePtr parseIf();
    ASTNodePtr parseWhile();
    ASTNodePtr parseFor();
    ASTNodePtr parseReturn();
    ASTNodePtr parseLocalDeclaration();

    ExprResult parseExpression();
    ExprResult parseAssignment();
    ExprResult parseLogicalOr();
    ExprResult parseLogicalAnd();
    ExprResult parseEquality();
    ExprResult parseRelational();
    ExprResult parseAdditive();
    ExprResult parseMultiplicative();
    ExprResult parseUnary();
    ExprResult parsePostfix();
    ExprResult parsePrimary();

    ExprResult makeLiteral(const Type& type, const std::string& text, bool numeric, double value);
    ExprResult ensureValue(const ExprResult& expr);
    ExprResult emitBinary(const ExprResult& left, const Token& op, const ExprResult& right);
    ExprResult emitUnary(const Token& op, const ExprResult& operand);
    void emitAssignment(const ExprResult& left, const ExprResult& right, const Token& opToken);
    bool checkCallArguments(const Symbol& function, const std::vector<ExprResult>& args, const Token& name);
};

#endif
