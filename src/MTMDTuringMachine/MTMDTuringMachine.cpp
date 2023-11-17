//
// Created by gruzi on 02/11/2023.
//

#include "MTMDTuringMachine.h"

MTMDTuringMachine::MTMDTuringMachine(const std::set<std::string> &tapeAlphabet,
                                     const std::set<std::string> &inputAlphabet,
                                     const TMTapes &tapes,
                                     const FiniteControl &control,
                                     void (*updateCallback) (
                                             const TMTapes &tapes,
                                             const TransitionDomain &domain, const TransitionImage &image)) :
                                     tapeAlphabet(tapeAlphabet), inputAlphabet(inputAlphabet),
                                     tapes(tapes), control(control),
                                     updateCallback(updateCallback)
                                      {

}
#include <iostream>
void MTMDTuringMachine::doTransition() {
    const std::vector<std::string> &currentSymbols = getCurrentTapeSymbols();
    const TransitionDomain domain(control.currentState, currentSymbols);
    const auto& foundDomain = control.transitions.find(domain);
    if(foundDomain != control.transitions.end()) {
        const TransitionImage &image = foundDomain->second;
        control.setCurrentState(image.state);
        for(unsigned int i=0;i<tapes.size();i++) {
            tapes[i]->replaceCurrentSymbol(image.replacementSymbols[i]);
            tapes[i]->moveTapeHead(image.directions[i]);
        }
        if(updateCallback) updateCallback(tapes, domain, image);
    }
}
std::vector<std::string> MTMDTuringMachine::getCurrentTapeSymbols() const {
    std::vector<std::string> currentSymbols;
    currentSymbols.reserve(tapes.size());
    for(const TMTape* currentTape : tapes) {
        currentSymbols.push_back(currentTape->getCurrentSymbol());
    }
    return currentSymbols;
}
