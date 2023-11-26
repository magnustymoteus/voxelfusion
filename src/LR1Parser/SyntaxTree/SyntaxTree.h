//

#ifndef VOXELFUSION_SYNTAXTREE_H
#define VOXELFUSION_SYNTAXTREE_H

#include <string>
#include <memory>
#include <vector>
#include "Lexer/Lexer.h"

struct STNode {
    const std::string label;
    const Token token;
    const std::vector<std::shared_ptr<STNode>> children;

    STNode(const std::string &label, const Token &token, const std::vector<std::shared_ptr<STNode>> &children);
    bool hasChildren() const;
    std::vector<std::shared_ptr<STNode>> getChildren() const;
    std::unique_ptr<Token> getToken() const;
};

#endif //VOXELFUSION_SYNTAXTREE_H
