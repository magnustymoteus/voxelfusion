//

#include "Lexer.h"
#include <iostream>
#include <regex>



bool matchesRegex(const std::string &str, const std::string &regex) {
    return std::regex_match(str, std::regex(regex));
}

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

std::vector<std::string> getSortedNonLiterals(const TokenType &type) {
    std::vector<std::string> nonLiterals = TokenMapping::nonLiterals.at(type);
    std::sort(nonLiterals.begin(), nonLiterals.end(), [](const std::string &a, const std::string &b) -> bool {
        return a.size() > b.size();
    });
    return nonLiterals;
}

std::vector<Token> Lexer::parseWord(const std::string &lexeme) const {
    std::vector<Token> result;
    if(!lexeme.empty()) {
        for(const auto &currentKeyword : getSortedNonLiterals(Token_Keyword)) {
            if(currentKeyword == lexeme) return {{Token_Keyword, lexeme}};
        }
        for(const auto &currentOperator : getSortedNonLiterals(Token_Operator)) {
            const size_t index = lexeme.find(currentOperator);
            if(index != std::string::npos) {
                const std::string prefix = lexeme.substr(0, index);
                const std::string suffix = lexeme.substr(
                        index+currentOperator.size(), lexeme.size()-index-currentOperator.size());
                for(const Token &currentPrefixToken : parseWord(prefix)) result.push_back(currentPrefixToken);
                result.emplace_back(Token_Operator, currentOperator);
                for(const Token &currentSuffixToken : parseWord(suffix)) result.push_back(currentSuffixToken);
                return result;
            }
        }

        for(const auto &currentPunctuator: getSortedNonLiterals(Token_Punctuator)) {
            const size_t index = lexeme.find(currentPunctuator);
            if(index != std::string::npos) {
                const std::string prefix = lexeme.substr(0, index);
                const std::string suffix = lexeme.substr(
                        index+currentPunctuator.size(), lexeme.size()-index-currentPunctuator.size());
                for(const Token &currentPrefixToken : parseWord(prefix)) result.push_back(currentPrefixToken);
                result.emplace_back(Token_Punctuator, currentPunctuator);
                for(const Token &currentSuffixToken : parseWord(suffix)) result.push_back(currentSuffixToken);
                return result;
            }
        }

        if(std::regex_match(lexeme, std::regex(TokenMapping::identifer.first))) return {{Token_Identifier, lexeme}};

        for(const auto &currentLiteral : TokenMapping::literals) {
            if(std::regex_match(lexeme, std::regex(currentLiteral.first))) return {{currentLiteral.second, lexeme}};
        }
    }
    return result;
}
void Lexer::tokenizeInput() {
    do {
        const std::string currentWord = getNextString();
        const std::vector<Token> nextTokens = parseWord(currentWord);
        for(const Token &currentToken : nextTokens) tokenizedInput.push_back(currentToken);
    } while(!reachedEnd());
    tokenizedInput.emplace_back(Token_EOS, "");
}

void Lexer::print() const {
    for(const Token &currentToken : tokenizedInput) {
        std::cout << "(" << currentToken.type << "," << currentToken.lexeme << ") ";
    }
    std::cout << std::endl;
}
std::vector<Token> Lexer::getTokenizedInput() const {
    return tokenizedInput;
}