//

#include "FiniteControl.h"

StatePointer findStartingState(const std::set<StatePointer> &states) {
    for(const auto & currentStatePtr : states) {
        if(currentStatePtr->isInitial) {
            return currentStatePtr;
        }
    }
    return nullptr;
}
FiniteControl::FiniteControl(const std::set<StatePointer> &states,
                             const std::unordered_map<std::string, std::map<std::vector<std::string>, TransitionImage>> & transitions)
        :  states(states),
           initialState(findStartingState(states)),
           currentState(initialState),
           transitions(transitions)
{}

#include <iostream>
FiniteControl::FiniteControl(const std::set<StatePointer> &states,
                             const std::map<TransitionDomain, TransitionImage> &transitionsArg) :
        states(states),
        initialState(findStartingState(states)),
        currentState(initialState) {
    for(const auto & currentTransition : transitionsArg) {
        transitions[currentTransition.first.state->name].insert({currentTransition.first.replacedSymbols, currentTransition.second});
    }
}

bool TransitionDomain::operator<(const TransitionDomain &other) const {
    if(state < other.state) return true;
    if(other.state < state) return false;
    if(replacedSymbols < other.replacedSymbols) return true;
    return false;
}


TransitionImage::TransitionImage(const StatePointer &state, const std::vector<std::string> &replacementSymbols,
                                 const std::vector<TMTapeDirection> &directionsArg) :
        state(state), replacementSymbols(replacementSymbols),
        directions(directionsArg.begin(), directionsArg.end())
{}