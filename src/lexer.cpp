#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string source, Diagnostics& diagnostics)
    : source(std::move(source)), diagnostics(diagnostics) {}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespaceCommentsAndPreprocessor();
        if (isAtEnd()) break;

        int startColumn = column;
        char c = advance();

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(identifier(startColumn, c));
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(number(startColumn, c));
        } else {
            switch (c) {
                case '"': tokens.push_back(stringLiteral(startColumn)); break;
                case '\'': tokens.push_back(charLiteral(startColumn)); break;
                case '+':
                    tokens.push_back(match('+') ? makeToken(TokenType::Increment, "++", startColumn)
                                                 : makeToken(TokenType::Plus, "+", startColumn));
                    break;
                case '-':
                    tokens.push_back(match('-') ? makeToken(TokenType::Decrement, "--", startColumn)
                                                 : makeToken(TokenType::Minus, "-", startColumn));
                    break;
                case '*': tokens.push_back(makeToken(TokenType::Star, "*", startColumn)); break;
                case '/': tokens.push_back(makeToken(TokenType::Slash, "/", startColumn)); break;
                case '%': tokens.push_back(makeToken(TokenType::Percent, "%", startColumn)); break;
                case '=':
                    tokens.push_back(match('=') ? makeToken(TokenType::Equal, "==", startColumn)
                                                 : makeToken(TokenType::Assign, "=", startColumn));
                    break;
                case '!':
                    tokens.push_back(match('=') ? makeToken(TokenType::NotEqual, "!=", startColumn)
                                                 : makeToken(TokenType::Not, "!", startColumn));
                    break;
                case '<':
                    tokens.push_back(match('=') ? makeToken(TokenType::LessEqual, "<=", startColumn)
                                                 : makeToken(TokenType::Less, "<", startColumn));
                    break;
                case '>':
                    tokens.push_back(match('=') ? makeToken(TokenType::GreaterEqual, ">=", startColumn)
                                                 : makeToken(TokenType::Greater, ">", startColumn));
                    break;
                case '&':
                    if (match('&')) tokens.push_back(makeToken(TokenType::And, "&&", startColumn));
                    else {
                        diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, "unexpected '&'; did you mean '&&'?");
                        tokens.push_back(makeToken(TokenType::Error, "&", startColumn));
                    }
                    break;
                case '|':
                    if (match('|')) tokens.push_back(makeToken(TokenType::Or, "||", startColumn));
                    else {
                        diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, "unexpected '|'; did you mean '||'?");
                        tokens.push_back(makeToken(TokenType::Error, "|", startColumn));
                    }
                    break;
                case '(': tokens.push_back(makeToken(TokenType::LeftParen, "(", startColumn)); break;
                case ')': tokens.push_back(makeToken(TokenType::RightParen, ")", startColumn)); break;
                case '{': tokens.push_back(makeToken(TokenType::LeftBrace, "{", startColumn)); break;
                case '}': tokens.push_back(makeToken(TokenType::RightBrace, "}", startColumn)); break;
                case '[': tokens.push_back(makeToken(TokenType::LeftBracket, "[", startColumn)); break;
                case ']': tokens.push_back(makeToken(TokenType::RightBracket, "]", startColumn)); break;
                case ',': tokens.push_back(makeToken(TokenType::Comma, ",", startColumn)); break;
                case ';': tokens.push_back(makeToken(TokenType::Semicolon, ";", startColumn)); break;
                default:
                    diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, std::string("unexpected character '") + c + "'");
                    tokens.push_back(makeToken(TokenType::Error, std::string(1, c), startColumn));
                    break;
            }
        }
    }

    tokens.push_back(Token{TokenType::End, "", line, column});
    return tokens;
}

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[pos];
}

char Lexer::peekNext() const {
    return pos + 1 >= source.size() ? '\0' : source[pos + 1];
}

char Lexer::advance() {
    char c = source[pos++];
    if (c == '\n') {
        ++line;
        column = 1;
    } else {
        ++column;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[pos] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespaceCommentsAndPreprocessor() {
    bool again = true;
    while (again && !isAtEnd()) {
        again = false;
        while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek()))) {
            advance();
        }

        if (peek() == '#') {
            while (!isAtEnd() && peek() != '\n') advance();
            again = true;
        } else if (peek() == '/' && peekNext() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            again = true;
        } else if (peek() == '/' && peekNext() == '*') {
            int startLine = line;
            int startColumn = column;
            advance();
            advance();
            while (!isAtEnd() && !(peek() == '*' && peekNext() == '/')) {
                advance();
            }
            if (isAtEnd()) {
                diagnostics.error(DiagnosticPhase::Lexical, startLine, startColumn, "unterminated block comment");
                return;
            }
            advance();
            advance();
            again = true;
        }
    }
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme, int startColumn) const {
    return Token{type, lexeme, line, startColumn};
}

Token Lexer::identifier(int startColumn, char first) {
    std::string lexeme(1, first);
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        lexeme += advance();
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KwInt},
        {"float", TokenType::KwFloat},
        {"char", TokenType::KwChar},
        {"bool", TokenType::KwBool},
        {"void", TokenType::KwVoid},
        {"if", TokenType::KwIf},
        {"else", TokenType::KwElse},
        {"while", TokenType::KwWhile},
        {"for", TokenType::KwFor},
        {"return", TokenType::KwReturn},
        {"true", TokenType::KwTrue},
        {"false", TokenType::KwFalse}
    };

    auto found = keywords.find(lexeme);
    if (found != keywords.end()) return makeToken(found->second, lexeme, startColumn);
    return makeToken(TokenType::Identifier, lexeme, startColumn);
}

Token Lexer::number(int startColumn, char first) {
    std::string lexeme(1, first);
    bool isFloat = false;

    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        lexeme += advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        isFloat = true;
        lexeme += advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            lexeme += advance();
        }
    }

    if (peek() == 'e' || peek() == 'E') {
        isFloat = true;
        lexeme += advance();
        if (peek() == '+' || peek() == '-') lexeme += advance();
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            diagnostics.error(DiagnosticPhase::Lexical, line, column, "expected digit after exponent");
        }
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            lexeme += advance();
        }
    }

    return makeToken(isFloat ? TokenType::FloatLiteral : TokenType::IntLiteral, lexeme, startColumn);
}

Token Lexer::stringLiteral(int startColumn) {
    std::string lexeme = "\"";
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            diagnostics.error(DiagnosticPhase::Lexical, line, column, "unterminated string literal");
            return makeToken(TokenType::Error, lexeme, startColumn);
        }
        if (peek() == '\\') {
            lexeme += advance();
            if (!isAtEnd()) lexeme += advance();
        } else {
            lexeme += advance();
        }
    }
    if (isAtEnd()) {
        diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, "unterminated string literal");
        return makeToken(TokenType::Error, lexeme, startColumn);
    }
    lexeme += advance();
    return makeToken(TokenType::StringLiteral, lexeme, startColumn);
}

Token Lexer::charLiteral(int startColumn) {
    std::string lexeme = "'";
    if (isAtEnd() || peek() == '\n') {
        diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, "unterminated char literal");
        return makeToken(TokenType::Error, lexeme, startColumn);
    }

    if (peek() == '\\') {
        lexeme += advance();
        if (!isAtEnd()) lexeme += advance();
    } else {
        lexeme += advance();
    }

    if (peek() != '\'') {
        diagnostics.error(DiagnosticPhase::Lexical, line, startColumn, "invalid char literal");
        return makeToken(TokenType::Error, lexeme, startColumn);
    }
    lexeme += advance();
    return makeToken(TokenType::CharLiteral, lexeme, startColumn);
}
