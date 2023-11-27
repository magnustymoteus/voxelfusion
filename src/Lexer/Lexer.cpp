//

#include "Lexer.h"
#include <iostream>
#include <regex>

#define IDENTIFIER_REGEX "^[A-z][A-z0-9]*$"
#define INTEGER_REGEX "^0$|^[1-9][0-9]*$"
#define DECIMAL_REGEX "^[+-]?([0-9]*[.])?[0-9]+$"

bool matchesRegex(const std::string &str, const std::string &regex) {
    return std::regex_match(str, std::regex(regex));
}

Lexer::Lexer(const std::string &input) : input(input), position(0)  {
    formatInput();
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
    if(!lexeme.empty()) {
        TokenType type;
        const auto &reservedFound = TokenMapping::reserved.find(lexeme);
        const auto &punctuationFound = TokenMapping::punctuation.find(lexeme);
        if (reservedFound != TokenMapping::reserved.end()) type = reservedFound->second;
        else if (punctuationFound != TokenMapping::punctuation.end()) type = punctuationFound->second;
        else {
            if(matchesRegex(lexeme, INTEGER_REGEX)) type = Token_Integer;
            else if(matchesRegex(lexeme, DECIMAL_REGEX)) type = Token_Decimal;
            else if(matchesRegex(lexeme, IDENTIFIER_REGEX)) type = Token_Identifier;
            else throw std::invalid_argument("Lexer cannot tokenize '"+lexeme+"'");
        }
        return {type, lexeme};
    }
    return {Token_EOS, lexeme};
}
Token Lexer::getNextToken() {
    const std::string lexeme = getNextString();
    return parseToken(lexeme);
}
void Lexer::formatInput() {
    for(unsigned int i=0;i<input.size();i++) {
        const std::string str = std::string(1, input[i]);
        const bool foundInPunctuation = TokenMapping::punctuation.find(str) != TokenMapping::punctuation.end();
        if(foundInPunctuation) {
            const bool hasNoSpaceBefore = i>0 && input[i-1] != ' ';
            const bool hasNoSpaceAfter = i<input.size()-1 && input[i+1] != ' ';
            if(hasNoSpaceBefore) {
                input.insert(input.begin()+i, ' ');
                i++;
            }
            if(hasNoSpaceAfter) {
                input.insert(input.begin()+i+1, ' ');
                i++;
            }
        }
    }
}
void Lexer::tokenizeInput() {
    TokenType currentType;
    do {
        Token currentToken = getNextToken();
        currentType = currentToken.type;
        tokenizedInput.push_back(currentToken);
    } while(currentType != Token_EOS);
}

void Lexer::print() const {
    for(const Token &currentToken : tokenizedInput) {
        std::cout << "(" << currentToken.type << "," << currentToken.lexeme << ") ";
    }
    std::cout << std::endl;
}
void Lexer::printFormattedInput() const {
    std::cout << input << std::endl;
}