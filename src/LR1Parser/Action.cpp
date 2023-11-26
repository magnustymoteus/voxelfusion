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

void Reduce::print() const {
    std::cout << "Reduce " << head << "->";
    CFGUtils::print(body);
}

void Shift::print() const {
    std::cout << "Shift " << index << std::endl;
}

void Accept::print() const {
    std::cout << "Accept\n";
}