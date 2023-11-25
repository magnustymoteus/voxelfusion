//

#ifndef CFG_CLR1PARSER_H
#define CFG_CLR1PARSER_H

#include "LR1Parser/LR1Parser.h"

class CLR1Parser : public LR1Parser {
public:
    explicit CLR1Parser(const std::string &jsonPath);
};


#endif //CFG_CLR1PARSER_H
