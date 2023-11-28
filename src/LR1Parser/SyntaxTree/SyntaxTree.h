//

#ifndef VOXELFUSION_SYNTAXTREE_H
#define VOXELFUSION_SYNTAXTREE_H

#include <string>
#include <memory>
#include <vector>
#include "Lexer/Lexer.h"

// TODO: export .txt graphviz file for visualization and debugging
struct STNode {
    const std::shared_ptr<Token> token;
    const std::string label;
    const std::vector<std::shared_ptr<STNode>> children;

    STNode(const std::string &label, const std::vector<std::shared_ptr<STNode>> &children)
    : label(label), children(children) {}
    explicit STNode(const Token &token) : token(std::make_shared<Token>(token)), label(token.lexeme) {}

    bool hasChildren() const {return !children.empty();}
    std::vector<std::shared_ptr<STNode>> getChildren() const {return children;}
    Token getToken() const {return *token;}
};

#endif //VOXELFUSION_SYNTAXTREE_H
