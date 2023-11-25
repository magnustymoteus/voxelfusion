//

#include "CLR1Parser.h"
#include "CFG/CFGUtils.h"


CLR1Parser::CLR1Parser(const std::string &jsonPath) : LR1Parser(jsonPath) {
    createItemSets();
    createParseTable();
}
