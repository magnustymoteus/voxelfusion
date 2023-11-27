//

#ifndef VOXELFUSION_LR1PARSER_H
#define VOXELFUSION_LR1PARSER_H


#include "LR1Parser/Action.h"
#include <memory>

class LR1ParseTableEntry {
public:
    // terminal : action (shift, reduce, or accept)
    std::map<std::string, std::unique_ptr<Action>> actionMap;
    std::map<std::string, unsigned int> gotoMap; // variable : item set index
};

typedef std::map<unsigned int, LR1ParseTableEntry> LR1ParseTable; // ItemSet Index : Row

class LR1Parser {
protected:
    std::vector<ItemSet> itemSets;
    std::map<unsigned int, std::map<std::string, unsigned int>> itemSetTransitionMap; // holds indices to item sets
    LR1ParseTable parseTable;
    const AugmentedCFG augmentedCfg;

    void createReduceAndAcceptActions();
    void createShiftActions();
    virtual void createGotos();

public:
    explicit LR1Parser(const std::string &jsonPath);

    std::shared_ptr<STNode> parse(const std::vector<Token> &tokenizedInput) const;

    virtual ~LR1Parser() = default;

    void createItemSets();
    void createParseTable();
    void print() const;
};


#endif //VOXELFUSION_LR1PARSER_H
