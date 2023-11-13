//

#ifndef CFG_CLR1PARSER_H
#define CFG_CLR1PARSER_H

#include "CFG/AugmentedCFG.h"

class CLR1Parser {
private:
    std::map<ItemSet, std::map<std::string, ItemSet>> itemSetTransitionMap;
public:
    explicit CLR1Parser(const CFG &cfg);
    [[nodiscard]] ItemSet getItemSet(const ItemSet &itemSet, const std::string &symbol) const;

};


#endif //CFG_CLR1PARSER_H
