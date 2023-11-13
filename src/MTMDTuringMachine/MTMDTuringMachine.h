//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
#define MTMDTURINGMACHINE_MTMDTURINGMACHINE_H

#include <set>
#include <vector>
#include <map>
#include <string>
#include "TMTape.h"


struct TransitionDomain {
    const std::string state;
    const std::vector<std::string> tapeSymbols;
    TransitionDomain(const std::string &state, const std::vector<std::string> &tapeSymbols)
    : state(state), tapeSymbols(tapeSymbols) {}
};
struct TransitionImage {
    const std::string state;
    const std::vector<std::string> tapeSymbols;
    const std::vector<TMTapeDirection> directions;
};

typedef std::map<TransitionDomain, TransitionImage> FiniteControl;
class MTMDTuringMachine {
public:
    std::set<std::string> states;
    std::set<std::string> acceptingStates;
    std::string startState;

    std::set<std::string> alphabet;
    std::set<std::string> tapeAlphabet;
    std::string blankSymbol;

    std::vector<TMTape> tapes;
    FiniteControl control;
};


#endif //MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
