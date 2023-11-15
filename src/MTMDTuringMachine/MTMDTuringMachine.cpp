//
// Created by gruzi on 02/11/2023.
//

#include "MTMDTuringMachine.h"

MTMDTuringMachine::MTMDTuringMachine(const std::set<State> &states, const State& startingState,
                                     const std::set<std::string> &inputAlphabet,
                                     const std::set<std::string> &tapeAlphabet, const TMTapes &tapes,
                                     const FiniteControl &control,
                                     void (*updateCallback) (
                                             const TMTapes &tapes,
                                             const TransitionImage &image))
                                     : states(states), startingState(startingState),
                                     currentState(std::make_shared<const State>(startingState)),
                                     inputAlphabet(inputAlphabet), tapeAlphabet(tapeAlphabet),
                                     tapes(tapes), control(control),
                                     updateCallback(updateCallback)
                                      {

}

