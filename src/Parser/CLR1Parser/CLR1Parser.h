//

#ifndef CFG_CLR1PARSER_H
#define CFG_CLR1PARSER_H

#include "CFG/AugmentedCFG.h"

class CLR1Parser {
private:
    std::vector<ItemSet> itemSets;
    std::map<unsigned int, std::map<std::string, unsigned int>> itemSetTransitionMap;
public:
    explicit CLR1Parser(const AugmentedCFG &);
};


#endif //CFG_CLR1PARSER_H
