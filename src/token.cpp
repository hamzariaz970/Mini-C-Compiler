#include "token.hpp"

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::End: return "end of file";
        case TokenType::Identifier: return "identifier";
        case TokenType::IntLiteral: return "integer literal";
        case TokenType::FloatLiteral: return "float literal";
        case TokenType::CharLiteral: return "char literal";
        case TokenType::StringLiteral: return "string literal";
        case TokenType::KwInt: return "int";
        case TokenType::KwFloat: return "float";
        case TokenType::KwChar: return "char";
        case TokenType::KwBool: return "bool";
        case TokenType::KwVoid: return "void";
        case TokenType::KwIf: return "if";
        case TokenType::KwElse: return "else";
        case TokenType::KwWhile: return "while";
        case TokenType::KwFor: return "for";
        case TokenType::KwReturn: return "return";
        case TokenType::KwTrue: return "true";
        case TokenType::KwFalse: return "false";
        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::Percent: return "%";
        case TokenType::Assign: return "=";
        case TokenType::Equal: return "==";
        case TokenType::NotEqual: return "!=";
        case TokenType::Less: return "<";
        case TokenType::LessEqual: return "<=";
        case TokenType::Greater: return ">";
        case TokenType::GreaterEqual: return ">=";
        case TokenType::And: return "&&";
        case TokenType::Or: return "||";
        case TokenType::Not: return "!";
        case TokenType::Increment: return "++";
        case TokenType::Decrement: return "--";
        case TokenType::LeftParen: return "(";
        case TokenType::RightParen: return ")";
        case TokenType::LeftBrace: return "{";
        case TokenType::RightBrace: return "}";
        case TokenType::LeftBracket: return "[";
        case TokenType::RightBracket: return "]";
        case TokenType::Comma: return ",";
        case TokenType::Semicolon: return ";";
        case TokenType::Error: return "error";
    }
    return "unknown";
}
