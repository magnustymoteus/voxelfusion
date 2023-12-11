#ifndef VOXELFUSION_TMGENERATOR_H
#define VOXELFUSION_TMGENERATOR_H


#include "../MTMDTuringMachine/MTMDTuringMachine.h"
#include "LR1Parser/SyntaxTree/SyntaxTree.h"

#include <memory>
#include <list>

using std::shared_ptr, std::set, std::string, std::map, std::list, std::vector;
#include <iostream>

struct PostponedTransition{
    PostponedTransition(const StatePointer& start, const StatePointer& end, const set<string>& leftOutSymbols={}, bool onlyTheseSymbols=false);
    PostponedTransition(const StatePointer &startState, int endLine, const set<string>& leftOutSymbols={}, bool onlyTheseSymbols=false);
    PostponedTransition(int startLine, const StatePointer &endState, const set<string>& leftOutSymbols={}, bool onlyTheseSymbols=false);
    PostponedTransition(int startLine, int endLine, const set<string>& leftOutSymbols={}, bool onlyTheseSymbols=false);

    StatePointer startState;
    StatePointer endState;
    int startLine;
    int endLine;
    set<string> leftOutSymbols;
    bool onlyTheseSymbols;
    vector<TMTapeDirection> directions = {TMTapeDirection::Stationary, TMTapeDirection::Stationary};
};

class TMGenerator {
    const set<string> &tapeAlphabet;
    map<TransitionDomain, TransitionImage>& transitions;
    set<StatePointer>& states;
    bool readableStateNames;
    map<int, StatePointer> lineStartStates;
    StatePointer currentLineBeginState;
    int currentStateNumber = 0;
    int currentLineNumber = 1;
    bool registerEndAsStartForNewLine = true;
    list<PostponedTransition> postponedTransitionBuffer;

    void explorer(const shared_ptr<STNode>& root);

    void registerRegularNewline(StatePointer& state);
    StatePointer makeState(int beginStateOfThisLineNumber = 0, bool accepting=false);

    TMTapeDirection parseDirection(const shared_ptr<STNode>& root);
    int parseInteger(const shared_ptr<STNode>& root);
    set<string> parseIdentifierList(const shared_ptr<STNode>& root) const;
    void identifierListPartRecursiveParser(const shared_ptr<STNode> &root, set<string> &output) const;
public:
    void assembleTasm(const shared_ptr<STNode> root);

    TMGenerator(const set<string> &tapeAlphabet, map<TransitionDomain, TransitionImage> &transitions,
                set<StatePointer> &states, bool readableStateNames = false);
};




#endif //VOXELFUSION_TMGENERATOR_H
