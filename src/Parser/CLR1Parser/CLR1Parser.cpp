//

#include "CLR1Parser.h"

#include "CFG/CFGUtils.h"

CLR1Parser::CLR1Parser(const AugmentedCFG &augmentedCfg) {
    CFGUtils::print(augmentedCfg.getItemSet());
}