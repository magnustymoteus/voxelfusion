//

#ifndef VOXELFUSION_TOKENTYPE_H
#define VOXELFUSION_TOKENTYPE_H


#include <unordered_map>
#include <string>
#include <unordered_set>

enum TokenType {
    Token_Identifier,
    Token_Integer,
    Token_Decimal,
    Token_True,
    Token_False,
    Token_Keyword,
    Token_Punctuator,
    Token_Operator,
    Token_EOS,
};

namespace TokenMapping {
    // lexer will copy these vectors and sort them according to their character length (so it checks "==" first instead of "=")
    const std::unordered_map<TokenType, std::vector<std::string>> nonLiterals = {
        {Token_Keyword, {"if",
                         "reading",
                         "while",
                         "accept",
                         "error",
                         "goto",
                         "move",
                         "write",
                         "into",
                         "until",
                         "current",
                         "left", "right", "up", "down", "forwards", "backwards",
                         "CA", "for", "run", "in"}},
        {Token_Operator,{"==", "=", "+=", "-=", "*=", "/="}},
        {Token_Punctuator, {"{","}",";", ",", "'"}}
    };
    const std::pair<std::string, std::string> identifer {"^[A-z][A-z0-9]*$", "Identifier"};

    const std::unordered_map<std::string, TokenType> literals = {
            {"^0$|^[1-9][0-9]*$", Token_Integer},
    };

    const std::unordered_map<TokenType, std::string> terminals = {
            {Token_Identifier, "Identifier"},
            {Token_Integer, "Integer"},
            {Token_EOS, "<EOS>"},
    };
}

#endif //VOXELFUSION_TOKENTYPE_H
