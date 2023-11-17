//#include "CFG/AugmentedCFG.h"
//#include "CFG/CFGUtils.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include <iostream>

using namespace std;
void printTransition(const TMTapes &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    std::cout << domain.state.name << " -> " << image.state.name << std::endl;
    for(const std::string &currentDomainSymbol : domain.replacedSymbols) {
        std::cout << currentDomainSymbol << " ";
    }
    std::cout << "replaced by ";
    for(const std::string &currentImageSymbol : image.replacementSymbols) {
        std::cout << currentImageSymbol << " ";
    }
    std::cout << std::endl;
    for(const TMTapeDirection &currentDirection : image.directions) {
        std::cout << "Move tapehead to direction " << static_cast<char>(currentDirection) << std::endl;
    }
}

int main() {
    /*AugmentedCFG aCfg("CFG/input/CFG.json");
    ItemSet nextItemSet;
    nextItemSet.insert({"T", aCfg.getItemSet().at("T")});
    CFGUtils::print(aCfg.getItemSet());
    std::cout << std::endl;
    CFGUtils::print(aCfg.computeClosure(nextItemSet));*/

    // example
    auto *tape2d = new TMTape2D();
    (*tape2d)[0][0].symbol = "C";
    (*tape2d)[1][3].symbol = "A";
    (*tape2d)[-1][-5].symbol = "C";
    (*tape2d)[-1][0].symbol= "L";
    tape2d->print();
    const StatePointer startState = std::make_shared<const State>("q0", true, false);
    const StatePointer state2  = std::make_shared<const State>("q1", false, false);

    std::set<StatePointer> states  = {startState, state2};
    FiniteControl control(states, {
        {
            TransitionDomain(startState, {"0"}),
            TransitionImage(state2, {"1"}, {Front})
        }});
    TMTapes tapes = {tape2d};
    MTMDTuringMachine tm({"0", "1"}, {"0", "1"}, tapes, control, printTransition);
    tm.doTransition();
    delete tape2d;
    return 0;
}