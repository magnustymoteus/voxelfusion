//

#ifndef VOXELFUSION_FINITECONTROL_H
#define VOXELFUSION_FINITECONTROL_H

#include <map>
#include <set>
#include <memory>

#include "TMTape.h"

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
    const State state;
    const std::vector<std::string> replacedSymbols;
    TransitionDomain(const StatePointer &state, const std::vector<std::string> &replacedSymbols) :
            state(*state), replacedSymbols(replacedSymbols) {}

    bool operator<(const TransitionDomain &other) const;
};
struct TransitionImage {
    /* Invariants:
     * |directions| = |replacingSymbols|
     * */
    const State state;
    const std::vector<std::string> replacementSymbols;
    const std::vector<TMTapeProbabilisticDirection> directions;
    TransitionImage(const StatePointer &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeProbabilisticDirection> &directions) :
            state(*state), replacementSymbols(replacementSymbols), directions(directions) {}
    TransitionImage(const StatePointer &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeDirection> &directionsArg);
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
