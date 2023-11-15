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

struct State {
    const bool &isInitial;
    const bool &isAccepting;
    const std::string &name;

    explicit State(const std::string &name, const bool &isInitial = false, const bool &isAccepting = false)
    : isInitial(isInitial), isAccepting(isAccepting), name(name) {}
    bool operator<(const State &other) const {return name < other.name;}
};

// TODO: invariants checker
// TODO: transitions
struct TransitionDomain {
    const State &state;
    const std::vector<std::string> &replacedSymbols;
    TransitionDomain(const State &state, const std::vector<std::string> &replacedSymbols) :
    state(state), replacedSymbols(replacedSymbols) {}
};
struct TransitionImage {
    /* Invariants:
     * |directions| = |replacingSymbols|
     * */
    const State &state;
    const std::vector<std::string> &replacementSymbols;
    const std::vector<TMTapeDirection> &directions;
    TransitionImage(const State &state,
                    const std::vector<std::string> &replacementSymbols,
                    const std::vector<TMTapeDirection> &directions) :
                    state(state), replacementSymbols(replacementSymbols), directions(directions) {}
};

typedef std::map<TransitionDomain, TransitionImage> FiniteControl;
typedef std::vector<TMTape> TMTapes;


/* Invariants:
* inputAlphabet is subset of tapeAlphabet
* tapeAlphabet always contains the blank symbol
* There is exactly one initial state in states set
* 'B' is reserved for the blank symbol
 * Amount of replacement and replaced symbols in every transition is equal to the amount of tapes
 * ...
* */
class MTMDTuringMachine {
private:
    const std::set<State> states;
    const State & startingState;
    std::shared_ptr<const State> currentState;

    const std::set<std::string> inputAlphabet;
    const std::string blankSymbol = "B";
    const std::set<std::string> tapeAlphabet;


    const std::vector<TMTape> tapes;
    const std::vector<TMTapeCell> tapeHeads;

    const FiniteControl &control;

    // called every transition (if not nullptr)
    // we only give image and not the domain in order to know what happened because we assume TM is deterministic
    void (*updateCallback) (const TMTapes &tapes, const TransitionImage &image);
public:

    MTMDTuringMachine(const std::set<State>& states,
                      const State &startingState,
                      const std::set<std::string> &inputAlphabet,
                      const std::set<std::string> &tapeAlphabet,
                      const TMTapes &tapes,
                      const FiniteControl &control,
  void (*updateCallback) (const TMTapes &tapes, const TransitionImage &image) = nullptr);
};


#endif //MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
