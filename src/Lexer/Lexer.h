//

#ifndef VOXELFUSION_LEXER_H
#define VOXELFUSION_LEXER_H

#include <vector>
#include "TokenType.h"


class Token {
public:
    const TokenType type;
    std::string lexeme; // a string that can represent an identifier,
    Token(const TokenType &type, const std::string &lexeme) : type(type), lexeme(lexeme) {}
};

class Lexer {
protected:
    std::string input;
    unsigned int position;
    std::vector<Token> tokenizedInput;

    char getCurrentChar() const;
    bool reachedEnd() const;
    Token parseToken(const std::string &lexeme) const;

    void advance();
    void skipWhitespace();
    void formatInput();
    void tokenizeInput();
    std::string getNextString();
    Token getNextToken();
public:
    explicit Lexer(const std::string &input);

    void print() const;
    void printFormattedInput() const;
};


#endif //VOXELFUSION_LEXER_H
