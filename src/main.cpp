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
    TMTapeCell3D currentCell("0");
    TMTape tape3d(currentCell);
    const StatePointer startState = std::make_shared<const State>("q0", true, false);
    const StatePointer state2  = std::make_shared<const State>("q1", false, false);

    std::set<std::shared_ptr<const State>> states  = {startState, state2};
    FiniteControl control(states, {
        {
            TransitionDomain(startState, {"0"}),
            TransitionImage(state2, {"1"}, {Up})
        }});
    TMTapes tapes = {tape3d};
    MTMDTuringMachine tm({"0", "1"}, {"0", "1"},tapes, control, printTransition);
    tm.doTransition();
    return 0;
}