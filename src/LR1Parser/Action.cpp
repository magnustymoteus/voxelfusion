//

#include "Action.h"
#include <iostream>
#include "CFG/CFGUtils.h"

void Reduce::operator()() {

}

void Shift::operator()() {

}

void Accept::operator()() {

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