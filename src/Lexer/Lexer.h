//

#ifndef VOXELFUSION_LEXER_H
#define VOXELFUSION_LEXER_H

#include <string>
#include <vector>

enum TokenType {
    Token_Identifier,
    Token_Integer,
    Token_EOS
};

class Token {
public:
    const TokenType type;
    const std::string lexeme; // a string that can represent an identifier,
    Token(const TokenType &type, const std::string &lexeme) : type(type), lexeme(lexeme) {}
};

class Lexer {
protected:
    const std::string input;
    unsigned int position;
    std::vector<Token> tokenizedInput;

    char getCurrentChar() const;
    void advance();
    Token parseInteger();
    Token parseIdentifier();
    void skipWhitespace();
    bool reachedEnd() const;
    void tokenizeInput();
    Token getNextToken();
public:
    explicit Lexer(const std::string &input);

    void print() const;
};


#endif //VOXELFUSION_LEXER_H
