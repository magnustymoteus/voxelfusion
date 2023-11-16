//

#ifndef CFG_CLR1PARSER_H
#define CFG_CLR1PARSER_H

#include "CFG/AugmentedCFG.h"

// TODO: generate item sets, ...
class CLR1Parser {
private:
    std::map<ItemSet, std::map<std::string, ItemSet>> itemSetTransitionMap;
public:
    explicit CLR1Parser(const AugmentedCFG &);
};


#endif //CFG_CLR1PARSER_H
