//

#ifndef VOXELFUSION_SYNTAXTREE_H
#define VOXELFUSION_SYNTAXTREE_H

#include <string>
#include <memory>
#include <vector>
#include "Lexer/Lexer.h"

struct STNode {
    const Token token;
    const std::vector<std::shared_ptr<STNode>> children;

    STNode(const Token &token, const std::vector<std::shared_ptr<STNode>> &children) :
     token(token), children(children) {}
    explicit STNode(const Token &token) : token(token) {}
    bool hasChildren() const {return !children.empty();}
    std::vector<std::shared_ptr<STNode>> getChildren() const {return children;}
    Token getToken() const {return token;}
};

#endif //VOXELFUSION_SYNTAXTREE_H
