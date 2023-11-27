//

#ifndef VOXELFUSION_LR1PARSINGSPACE_H
#define VOXELFUSION_LR1PARSINGSPACE_H

#include "SyntaxTree/SyntaxTree.h"
#include <stack>
#include <memory>

class LR1ParsingSpace {
public:
    std::stack<std::shared_ptr<STNode>> nodeStack;
    std::stack<unsigned int> stateStack;
    bool accepted;
    unsigned int tokenIndex;
    LR1ParsingSpace() : accepted(false), stateStack({0}), tokenIndex(0) {}
};

#endif //VOXELFUSION_LR1PARSINGSPACE_H
