//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
#define MTMDTURINGMACHINE_MTMDTURINGMACHINE_H

#include "FiniteControl.h"
#include <iostream>
#include "invariants.h"


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
protected:
    const std::set<std::string> tapeAlphabet;
    const std::set<std::string> inputAlphabet;

    std::tuple<TMTapeType*...> tapes;
    const unsigned int tapeCount;

    FiniteControl control;


    bool hasAccepted;

    void (*updateCallback) (const std::tuple<TMTapeType*...> &, const std::vector<unsigned int>);
public:
    bool isHalted;
    MTMDTuringMachine(const std::set<std::string> &tapeAlphabet,
                      const std::set<std::string> &inputAlphabet,
                      const std::tuple<TMTapeType*...> &tapes,
                      const FiniteControl &control,
  void (*updateCallback) (const std::tuple<TMTapeType*...> &, const std::vector<unsigned int>) = nullptr) :
            tapeAlphabet(tapeAlphabet), inputAlphabet(inputAlphabet),
            tapes(tapes), tapeCount(sizeof...(TMTapeType)), control(control),
            updateCallback(updateCallback),
            isHalted(false), hasAccepted(false){
        static_assert(std::conjunction<std::is_base_of<TMTape,TMTapeType>...>(), "TM must only be given tapes!");
    }
    std::tuple<TMTapeType*...> getTapes() const {return tapes;}

  void doTransition() {
      PRECONDITION(!isHalted);
      const std::vector<std::string> &currentSymbols = getCurrentTapeSymbols();
      const auto foundStateTransitions = control.transitions.find(control.currentState);
      if (foundStateTransitions != control.transitions.end()) {
          const auto &domain = foundStateTransitions->second;
          const auto &foundDomain = [&]() {
              const auto foundExactMatch = domain.find(currentSymbols);
              if (foundExactMatch != domain.end()) return foundExactMatch;
              for (auto iter = domain.begin(); iter != domain.end(); iter++) {
                  for (int i = 0; i < iter->first.size(); i++) {
                      if (currentSymbols[i] != iter->first[i] && iter->first[i] != SYMBOL_ANY) break;
                      if (i == iter->first.size() - 1) return iter;
                  }
              }
              return domain.end();
          }();
          if (foundDomain != domain.end()) {

              const TransitionImage &image = foundDomain->second;
              control.setCurrentState(image.state);
              unsigned int i = 0;
              std::vector<unsigned int> changedTapesIndex;
              std::apply([&](auto &&... currentTape) {
                  (((currentTape->getCurrentSymbol() != image.replacementSymbols[i]) &&
                    changedTapesIndex.emplace_back(i),
                          currentTape->replaceCurrentSymbol(image.replacementSymbols[i]),
                          currentTape->moveTapeHead(image.directions[i]()),
                          i++
                  ), ...);
              }, tapes);
              if (control.currentState->type != State_NonHalting) {
                  isHalted = true;
                  if (control.currentState->type == State_Accepting) hasAccepted = true;
              }
              if (updateCallback) updateCallback(tapes, changedTapesIndex);
          } else isHalted = true;
      }
  }

  [[nodiscard]] std::vector<std::string> getCurrentTapeSymbols() const {
      std::vector<std::string> currentSymbols;
      currentSymbols.reserve(tapeCount);
      std::apply([&currentSymbols](const auto&... currentTape) {
          ((currentSymbols.push_back(currentTape->getCurrentSymbol())), ...);}, tapes);
      return currentSymbols;
    }

    void doTransitions(const int &steps=-1) {
        int i = 0;
        const bool definite = (steps >= 0);
        while((!definite && !isHalted) || (definite && i<steps)) {
            doTransition();
            //std::cout << "---------------" << std::endl;
           // std::get<1>(tapes)->print();
           // std::cout << getFiniteControl().currentState->name << std::endl;
            i++;
        }
    }
    const FiniteControl& getFiniteControl(){
        return control;
    }
};



#endif //MTMDTURINGMACHINE_MTMDTURINGMACHINE_H
