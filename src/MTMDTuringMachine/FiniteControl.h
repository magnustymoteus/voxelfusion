//

#ifndef VOXELFUSION_FINITECONTROL_H
#define VOXELFUSION_FINITECONTROL_H

#include <map>
#include <set>
#include <memory>

#include "TMTape.h"

struct State {
    const bool isInitial;
    const bool isAccepting;
    const std::string name;

    explicit State(const std::string &name, const bool &isInitial = false, const bool &isAccepting = false)
            : isInitial(isInitial), isAccepting(isAccepting), name(name) {}
    bool operator<(const State &other) const {return name < other.name;}
};
typedef std::shared_ptr<const State> StatePointer;

struct TransitionDomain {
    const State state;
    const std::vector<std::string> replacedSymbols;
    TransitionDomain(const StatePointer &state, const std::vector<std::string> &replacedSymbols) :
            state(*state), replacedSymbols(replacedSymbols) {}

    bool operator<(const TransitionDomain &other) const {return state < other.state;}
};
struct TransitionImage {
    /* Invariants:
     * |directions| = |replacingSymbols|
     * */
    const State state;
    const std::vector<std::string> replacementSymbols;
    const std::vector<TMTapeDirection> directions;
    TransitionImage(const StatePointer &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeDirection> &directions) :
            state(*state), replacementSymbols(replacementSymbols), directions(directions) {}
};


class FiniteControl {
public:
    const std::set<StatePointer> states;
    const StatePointer initialState;
    StatePointer currentState;

    std::map<TransitionDomain, TransitionImage> transitions;

    FiniteControl(const std::set<StatePointer> &states, const std::map<TransitionDomain, TransitionImage> &transitions);
    void setCurrentState(const State &newCurrentState) {currentState = std::make_shared<const State>(newCurrentState);}
};


#endif //VOXELFUSION_FINITECONTROL_H
