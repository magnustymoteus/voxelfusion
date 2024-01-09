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
    int tape = 0;
    string toWrite;
    vector<TMTapeDirection> directions = {TMTapeDirection::Stationary, TMTapeDirection::Stationary, TMTapeDirection::Stationary};
};

class TMGenerator {
    set<string> &tapeAlphabet;
    map<TransitionDomain, TransitionImage>& transitions;
    set<StatePointer>& states;
    bool readableStateNames;
    map<int, StatePointer> lineStartStates;
    StatePointer currentLineBeginState;
    int currentStateNumber = 0;
    int currentLineNumber = 1;
    bool registerEndAsStartForNewLine = true;
    list<PostponedTransition> postponedTransitionBuffer;
    inline static const string VariableTapeStart = "VTB";
    inline static const string VariableTapeEnd = "VTE";
    StatePointer CAstart;
    StatePointer CAend;

    void alphabetExplorer(const shared_ptr<STNode>& root);
    void explorer(const shared_ptr<STNode>& root);

    void registerRegularNewline(StatePointer& state);
    StatePointer makeState(int beginStateOfThisLineNumber = 0, bool accepting=false);
    StatePointer MoveToVariableMarker(StatePointer startState, const string &variableName);

    static TMTapeDirection parseDirection(const shared_ptr<STNode>& root);
    static int parseInteger(const shared_ptr<STNode>& root);
    static string parseSymbolLiteral(const shared_ptr<STNode>& root);
    static set<string> parseIdentifierList(const shared_ptr<STNode>& root);
    static void identifierListPartRecursiveParser(const shared_ptr<STNode> &root, set<string> &output);
    string IntegerAsBitString(int in, bool flipped = false);
public:
    void assembleTasm(const shared_ptr<STNode> root);

    TMGenerator(set<string> &tapeAlphabet, map<TransitionDomain, TransitionImage> &transitions,
                set<StatePointer> &states, bool readableStateNames = false);

    StatePointer copyIntegerToThirdTape(StatePointer startState, const string &variableName);

    void addThirdToSecond(vector<StatePointer> &writeValueStates, bool subtract);

    void currentIntoVariable(const string &variableName, const StatePointer &beginState,
                             const StatePointer &destination);

    void tapeMove(TMTapeDirection direction, StatePointer &beginState, StatePointer &destination);

    void immediateAddition(const string &variableName, string &binaryAddedValue, StatePointer &startingState,
                           StatePointer &destination);

    void
    IntegerCompare(const string &variableName, string &binaryComparedValue, StatePointer &standardDestination,
                   int conditionalDestinationLineNumber, StatePointer beginState, StatePointer conditionalEndState = nullptr);

    void integerAssignment(const string &variableName, string &binaryAssignedValue, StatePointer &beginState,
                            StatePointer &destination);
};




#endif //VOXELFUSION_TMGENERATOR_H
