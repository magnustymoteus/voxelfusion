//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
#define MTMDTURINGMACHINE_MTMDTURINGMACHINE_H

#include "FiniteControl.h"
#include <iostream>


/* Invariants:
* inputAlphabet is subset of tapeAlphabet
* tapeAlphabet always contains the blank symbol
* There is exactly one initial state in states set
* 'B' is reserved for the blank symbol
 * Amount of replacement and replaced symbols in every transition is equal to the amount of tapes
 * ...
* */
// TODO: invariants (pre- and post- conditions), halting states, testing for multi tape scenarios
template<class ...TMTapeType>
class MTMDTuringMachine {
private:
    const std::set<std::string> tapeAlphabet;
    const std::set<std::string> inputAlphabet;

    std::tuple<TMTapeType*...> tapes;
    const unsigned int tapeCount;

    FiniteControl control;

    // called every transition (if it's not nullptr)
    void (*updateCallback) (const std::tuple<TMTapeType*...> &tapes,
            const TransitionDomain &domain, const TransitionImage &image);
public:
    MTMDTuringMachine(const std::set<std::string> &tapeAlphabet,
                      const std::set<std::string> &inputAlphabet,
                      const std::tuple<TMTapeType*...> &tapes,
                      const FiniteControl &control,
  void (*updateCallback) (const std::tuple<TMTapeType*...> &tapes,
          const TransitionDomain &domain, const TransitionImage &image) = nullptr) :
            tapeAlphabet(tapeAlphabet), inputAlphabet(inputAlphabet),
            tapes(tapes), tapeCount(sizeof...(TMTapeType)), control(control),

            updateCallback(updateCallback){
        static_assert(std::conjunction<std::is_base_of<TMTape,TMTapeType>...>(), "TM was not given tapes!");
    }
    std::tuple<TMTapeType*...> getTapes() const {return tapes;}
  void doTransition() {
      const std::vector<std::string> &currentSymbols = getCurrentTapeSymbols();
      const TransitionDomain domain(control.currentState, currentSymbols);
      const auto& foundDomain = control.transitions.find(domain);
      if(foundDomain != control.transitions.end()) {
          const TransitionImage &image = foundDomain->second;
          control.setCurrentState(image.state);
          int i=0;
            std::apply([&](auto&&... currentTape){
              ((currentTape->replaceCurrentSymbol(image.replacementSymbols[i]),
                      currentTape->moveTapeHead(image.directions[i]),
                     i++), ...);
              }, tapes);
         if(updateCallback) updateCallback(tapes, domain, image);
     }
  }

  [[nodiscard]] std::vector<std::string> getCurrentTapeSymbols() const {
      std::vector<std::string> currentSymbols;
      currentSymbols.reserve(tapeCount);
      std::apply([&currentSymbols](auto&&... currentTape) {
          ((currentSymbols.push_back(currentTape->getCurrentSymbol())), ...);}, tapes);
      return currentSymbols;
    }
};


#endif //MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
