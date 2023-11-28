//

#include "Action.h"
#include <iostream>
#include "CFG/CFGUtils.h"

void Reduce::operator()(LR1ParsingSpace &parsingSpace) const {
}

void Shift::operator()(LR1ParsingSpace &parsingSpace) const {

}

void Accept::operator()(LR1ParsingSpace &parsingSpace) const {

}

void Action::print() const {
    std::cout << getString() << std::endl;
}

std::string Reduce::getString() const {
    return "Reduce " + head + "->" + CFGUtils::getString(body);
}

std::string Shift::getString() const {
    return "Shift " + std::to_string(index);
}

std::string Accept::getString() const {
    return "Accept";
}