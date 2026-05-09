#ifndef MINIC_TOKEN_HPP
#define MINIC_TOKEN_HPP

#include <string>

enum class TokenType {
    End,
    Identifier,
    IntLiteral,
    FloatLiteral,
    CharLiteral,
    StringLiteral,

    KwInt,
    KwFloat,
    KwChar,
    KwBool,
    KwVoid,
    KwIf,
    KwElse,
    KwWhile,
    KwFor,
    KwReturn,
    KwTrue,
    KwFalse,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Assign,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or,
    Not,
    Increment,
    Decrement,

    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Semicolon,

    Error
};

struct Token {
    TokenType type = TokenType::End;
    std::string lexeme;
    int line = 1;
    int column = 1;
};

std::string tokenTypeName(TokenType type);

#endif
