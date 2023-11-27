//

#include "Lexer.h"
#include <iostream>
#include <regex>

#define IDENTIFIER_REGEX "^[A-z][A-z0-9]*$"
#define INTEGER_REGEX "^0$|^[1-9][0-9]*$"
#define DECIMAL_REGEX "^[+-]?([0-9]*[.])?[0-9]+$"

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
std::string Lexer::getNextString() {
    skipWhitespace();
    std::string result;
    while(!reachedEnd() && !std::isspace(getCurrentChar())) {
        result += getCurrentChar();
        advance();
    }
    return result;
}
Token Lexer::parseToken(const std::string &lexeme) const {

}
Token Lexer::getNextToken() {
    return parseToken(getNextString());
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