//

#ifndef VOXELFUSION_TMTAPECELL_H
#define VOXELFUSION_TMTAPECELL_H

#include <string>

struct TMTapeCell {
    std::string symbol;
    explicit TMTapeCell(const std::string &symbolArg) : symbol(symbolArg) {}
    explicit TMTapeCell(std::string &&symbolArg) : symbol(std::move(symbolArg)) {}
    TMTapeCell() : symbol("B") {}
};


#endif //VOXELFUSION_TMTAPECELL_H
