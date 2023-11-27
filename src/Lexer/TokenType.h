//

#ifndef VOXELFUSION_TOKENTYPE_H
#define VOXELFUSION_TOKENTYPE_H


#include <unordered_map>
#include <string>

enum TokenType {
    Token_Identifier,
    Token_Integer,
    Token_Decimal,

    Token_LParen,
    Token_RParen,
    Token_LBrace,
    Token_RBrace,

    Token_Equal,
    Token_Plus,
    Token_Minus,
    Token_Multiply,
    Token_Divide,
    Token_Modulo,

    Token_Semicolon,

    Token_BoolType,
    Token_IntType,
    Token_DecType,

    Token_If,
    Token_Else,

    Token_While,

    Token_And,
    Token_Or,
    Token_Equality,
    Token_Inequality,
    Token_Greater,
    Token_Lesser,
    Token_GreaterEq,
    Token_LesserEq,

    Token_True,
    Token_False,

    Token_EOS
};

std::unordered_map<std::string, TokenType> reserved = {
        {"eq", Token_Equality},
        {"neq", Token_Inequality},
        {"and", Token_And},
        {"or", Token_Or},
        {">", Token_Greater},
        {"<", Token_Lesser},
        {">=", Token_GreaterEq},
        {"<=", Token_LesserEq},
        {"true", Token_True},
        {"false", Token_False},
        {"if", Token_If},
        {"else", Token_Else},
        {"while", Token_While},
        {"bool", Token_BoolType},
        {"int", Token_IntType},
        {"dec", Token_DecType},
        {"=", Token_Equal},
        {"+", Token_Plus},
        {"-", Token_Minus},
        {"*", Token_Multiply},
        {"/", Token_Divide},
        {"%", Token_Modulo}
};
std::unordered_map<char, TokenType> punctation = {
        {'(', Token_LParen},
        {')', Token_RParen},
        {'{', Token_LBrace},
        {'}', Token_RBrace},
        {';', Token_Semicolon},
};


#endif //VOXELFUSION_TOKENTYPE_H
