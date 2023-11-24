//

#ifndef CFG_CLR1PARSER_H
#define CFG_CLR1PARSER_H

#include "CFG/AugmentedCFG.h"
#include "LR1Parser/Action.h"
#include <memory>
#include <stack>


class LR1ParseTableEntry {
public:
    // terminal : action (shift, reduce, or accept)
    std::map<std::string, std::unique_ptr<Action>> actionMap;
    std::map<std::string, unsigned int> gotoMap; // variable : item set index
};

typedef std::map<unsigned int, LR1ParseTableEntry> LR1ParseTable; // ItemSet Index : Row

class LR1ParsingSpace {
public:
    std::stack<unsigned int> stack;
    std::vector<std::string> symbols, input;
};
class CLR1Parser {
private:
    std::vector<ItemSet> itemSets;
    std::map<unsigned int, std::map<std::string, unsigned int>> itemSetTransitionMap; // holds indices to item sets
    LR1ParseTable parseTable;
    const AugmentedCFG augmentedCfg;

    void createReduceAndAcceptActions();
    void createShiftActions();
    void createGotos();

public:
    void createItemSets();
    void createParseTable();

    explicit CLR1Parser(const std::string &jsonPath);
};


#endif //CFG_CLR1PARSER_H
