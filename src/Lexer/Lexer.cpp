//

#include "Lexer.h"
#include <iostream>

Lexer::Lexer(const std::string &input) : input(input), position(0)  {
    tokenizeInput();
}

void Lexer::advance() {
    position++;
}
char Lexer::getCurrentChar() const {
    return input[position];
}
void Lexer::skipWhitespace() {
    while(std::isspace(getCurrentChar())) advance();
}
bool Lexer::reachedEnd() const {
    return position > input.size()-1;
}
Token Lexer::parseInteger() {
    std::string lexeme;
    while(std::isdigit(getCurrentChar())) {
        lexeme += getCurrentChar();
        advance();
    }
    return {Token_Integer, lexeme};
}
Token Lexer::parseIdentifier() {
    std::string lexeme;
    while(std::isgraph(getCurrentChar())) {
        lexeme += getCurrentChar();
        advance();
    }
    return {Token_Identifier, lexeme};
}
Token Lexer::getNextToken() {
   while(!reachedEnd()) {
       skipWhitespace();
        if(std::isdigit(getCurrentChar())) return parseInteger();
        else if(std::isgraph(getCurrentChar())) return parseIdentifier();
    }
    return {Token_EOS, ""};
}
void Lexer::tokenizeInput() {
    TokenType currentType;
    do {
        Token currentToken = getNextToken();
        currentType = currentToken.type;
        if(currentType != Token_EOS) tokenizedInput.push_back(currentToken);
    } while(currentType != Token_EOS);
}

void Lexer::print() const {
    for(const Token &currentToken : tokenizedInput) {
        std::cout << "(" << currentToken.type << "," << currentToken.lexeme << ") ";
    }
    std::cout << std::endl;
}