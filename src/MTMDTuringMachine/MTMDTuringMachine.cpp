//
// Created by gruzi on 02/11/2023.
//

//#include "MTMDTuringMachine.h"

/*template<class ...TMTapeType>
MTMDTuringMachine<TMTapeType...>::MTMDTuringMachine(const std::set<std::string> &tapeAlphabet,
                                     const std::set<std::string> &inputAlphabet,
                                     const std::tuple<TMTapeType...> &tapes,
                                     const FiniteControl &control,
                                     void (*updateCallback) (
                                             const std::tuple<TMTapeType...> &tapes,
                                             const TransitionDomain &domain, const TransitionImage &image)) :
                                     tapeAlphabet(tapeAlphabet), inputAlphabet(inputAlphabet),
                                     tapes(tapes), control(control),
                                     updateCallback(updateCallback)
                                      {

}
template<class ...TMTapeType>
void MTMDTuringMachine<TMTapeType...>::doTransition() {
    const std::vector<std::string> &currentSymbols = getCurrentTapeSymbols();
    const TransitionDomain domain(control.currentState, currentSymbols);
    const auto& foundDomain = control.transitions.find(domain);
    if(foundDomain != control.transitions.end()) {
        const TransitionImage &image = foundDomain->second;
        control.setCurrentState(image.state);
        unsigned int i = 0;
        for(TMTape* &currentTape : tapes) {
            currentTape->replaceCurrentSymbol(image.replacementSymbols[i]);
            currentTape->moveTapeHead(image.directions[i]);
            i++;
        }
        //for(unsigned int i=0;i<image.replacementSymbols.size();i++) {
            //tapes[i]->replaceCurrentSymbol(image.replacementSymbols[i]);
           // tapes[i]->moveTapeHead(image.directions[i]);
        //}
        if(updateCallback) updateCallback(tapes, domain, image);
    }
}
template<class ...TMTapeType>
std::vector<std::string> MTMDTuringMachine<TMTapeType...>::getCurrentTapeSymbols() const {
    std::vector<std::string> currentSymbols;
    currentSymbols.reserve(tapes.size());
    for(const TMTape* currentTape : tapes) {
        currentSymbols.push_back(currentTape->getCurrentSymbol());
    }
    return currentSymbols;
}
*/