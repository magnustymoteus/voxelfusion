//

#ifndef VOXELFUSION_FINITECONTROL_H
#define VOXELFUSION_FINITECONTROL_H

#include <unordered_map>
#include <map>
#include <set>
#include <memory>

#include "TMTape.h"
#include "SymbolTrie/SymbolTrie.h"

enum StateType {State_NonHalting, State_Accepting, State_Rejecting};

struct State {
    const StateType type;
    const bool isInitial;
    const std::string name;

    explicit State(const std::string &name, const bool &isInitial = false, const StateType &type = State_NonHalting)
            : isInitial(isInitial), type(type), name(name) {}
    bool operator<(const State &other) const {return name < other.name;}
};
typedef std::shared_ptr<const State> StatePointer;

struct TransitionDomain {
    const StatePointer state;
    const std::vector<std::string> replacedSymbols;
    TransitionDomain(const StatePointer &state, const std::vector<std::string> &replacedSymbols) :
            state(state), replacedSymbols(replacedSymbols) {}

    bool operator<(const TransitionDomain &other) const;
};

struct TransitionImage {
    /* Invariants:
     * |directions| = |replacingSymbols|
     * */
    const StatePointer state;
    const std::vector<std::string> replacementSymbols;
    const std::vector<TMTapeProbabilisticDirection> directions;
    TransitionImage(const StatePointer &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeProbabilisticDirection> &directions) :
            state(state), replacementSymbols(replacementSymbols), directions(directions) {}
    TransitionImage(const StatePointer &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeDirection> &directionsArg);
};


typedef std::map<std::vector<std::string>, TransitionImage> StateTransitions;
class FiniteControl {
public:
    const std::set<StatePointer> states;
    const StatePointer initialState;
    StatePointer currentState;

    std::unordered_map<StatePointer, Trie<TransitionImage>> transitions;

    FiniteControl(const std::set<StatePointer> &states, const std::map<TransitionDomain, TransitionImage> &transitions);
    FiniteControl(const std::set<StatePointer> &states, const std::unordered_map<StatePointer, Trie<TransitionImage>> &transitions);
    void setCurrentState(const StatePointer &newCurrentState) {currentState = newCurrentState;}
};

#endif //VOXELFUSION_FINITECONTROL_H
