#ifndef MINIC_LEXER_HPP
#define MINIC_LEXER_HPP

#include "diagnostics.hpp"
#include "token.hpp"

#include <string>
#include <vector>

class Lexer {
public:
    Lexer(std::string source, Diagnostics& diagnostics);
    std::vector<Token> scanTokens();

private:
    std::string source;
    Diagnostics& diagnostics;
    std::size_t pos = 0;
    int line = 1;
    int column = 1;

    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);

    void skipWhitespaceCommentsAndPreprocessor();
    Token makeToken(TokenType type, const std::string& lexeme, int startColumn) const;
    Token identifier(int startColumn, char first);
    Token number(int startColumn, char first);
    Token stringLiteral(int startColumn);
    Token charLiteral(int startColumn);
};

#endif
