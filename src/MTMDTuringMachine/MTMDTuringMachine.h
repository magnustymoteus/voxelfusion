//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
#define MTMDTURINGMACHINE_MTMDTURINGMACHINE_H

#include "FiniteControl.h"


/* Invariants:
* inputAlphabet is subset of tapeAlphabet
* tapeAlphabet always contains the blank symbol
* There is exactly one initial state in states set
* 'B' is reserved for the blank symbol
 * Amount of replacement and replaced symbols in every transition is equal to the amount of tapes
 * ...
* */
// TODO: invariants (pre- and post- conditions), halting states
class MTMDTuringMachine {
private:
    const std::string blankSymbol = "B";
    const std::set<std::string> tapeAlphabet;
    const std::set<std::string> inputAlphabet;

    std::vector<TMTape> tapes;
    FiniteControl control;

    // called every transition (if it's not nullptr)
    void (*updateCallback) (const TMTapes &tapes, const TransitionDomain &domain, const TransitionImage &image);
public:

    MTMDTuringMachine(const std::set<std::string> &tapeAlphabet,
                      const std::set<std::string> &inputAlphabet,
                      TMTapes &tapes,
                      FiniteControl &control,
  void (*updateCallback) (const TMTapes &tapes, const TransitionDomain &domain, const TransitionImage &image) = nullptr);

  void doTransition();

  [[nodiscard]] std::vector<std::string> getCurrentTapeSymbols() const;
};


#endif //MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
