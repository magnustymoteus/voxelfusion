#ifndef VOXELFUSION_TMGENERATOR_H
#define VOXELFUSION_TMGENERATOR_H


#include "../MTMDTuringMachine/MTMDTuringMachine.h"
#include "LR1Parser/SyntaxTree/SyntaxTree.h"

#include <memory>

using std::shared_ptr, std::make_shared, std::set, std::tuple, std::string, std::map, std::to_string,
        std::cout, std::endl, std::runtime_error;
#include <iostream>

class TMGenerator {
    const set<string> &tapeAlphabet;
    map<TransitionDomain, TransitionImage>& transitions;
    set<StatePointer>& states;
    map<int, StatePointer> lineStates;
    StatePointer currentState;
    int currentStateNumber = 0;

    void explorer(const shared_ptr<STNode>& root);

    void registerFollowingBeginState(StatePointer& state);

    TMTapeDirection parseDirection(const shared_ptr<STNode>& root);
    int parseInteger(const shared_ptr<STNode>& root);
public:
    void assembleTasm(const shared_ptr<STNode> root);

    TMGenerator(const set<string> &tapeAlphabet,
                map<TransitionDomain, TransitionImage> &transitions, set<StatePointer> &states);
};




#endif //VOXELFUSION_TMGENERATOR_H
