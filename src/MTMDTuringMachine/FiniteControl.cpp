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
                             const std::map<TransitionDomain, TransitionImage> &transitions) :
        states(states),
        initialState(findStartingState(states)),
        currentState(findStartingState(states)),
        transitions(transitions) {}