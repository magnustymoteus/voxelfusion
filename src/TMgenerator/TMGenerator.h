#ifndef VOXELFUSION_TMGENERATOR_H
#define VOXELFUSION_TMGENERATOR_H


#include "../MTMDTuringMachine/MTMDTuringMachine.h"
#include "LR1Parser/SyntaxTree/SyntaxTree.h"

#include <memory>
#include <list>
#include <map>

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
    vector<TMTapeDirection> directions = {TMTapeDirection::Stationary, TMTapeDirection::Stationary, TMTapeDirection::Stationary, TMTapeDirection::Stationary};
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
    list<PostponedTransition> postponedTransitionBuffer;
    inline static const string VariableTapeStart = "VTB";
    inline static const string VariableTapeEnd = "VTE";
    StatePointer CAstart;
    StatePointer CAend;

    void alphabetExplorer(const shared_ptr<STNode>& root);
    void explorer(const shared_ptr<STNode>& root);

    void registerRegularNewline(StatePointer& state);
    StatePointer makeState(int beginStateOfThisLineNumber = 0, bool accepting=false);
    StatePointer MoveToVariableValue(StatePointer startState, const string &variableName,
                                     const string &variableContainingIndex="");

    static TMTapeDirection parseDirection(const shared_ptr<STNode>& root);
    static int parseInteger(const shared_ptr<STNode>& root);
    static std::pair<string, string> parseVariableLocationContainer(const shared_ptr<STNode> &root);
    static string parseSymbolLiteral(const shared_ptr<STNode>& root);
    static set<string> parseIdentifierList(const shared_ptr<STNode>& root);
    static void identifierListPartRecursiveParser(const shared_ptr<STNode> &root, set<string> &output);
    string IntegerAsBitString(int in, bool flipped = false);

    StatePointer getNextLineStartState();
public:
    void assembleTasm(const shared_ptr<STNode> root);

    TMGenerator(set<string> &tapeAlphabet, map<TransitionDomain, TransitionImage> &transitions,
                set<StatePointer> &states, bool readableStateNames = false);

    StatePointer copyIntegerToThirdTape(StatePointer startState, bool backToStart);

    void addThirdToSecond(vector<StatePointer> &writeValueStates, bool subtract);

    void currentIntoVariable(const string &variableName, const StatePointer &beginState,
                             const StatePointer &destination, int tapeIndex,
                             const string &variableContainingIndex="");

    void tapeMove(TMTapeDirection direction, StatePointer &beginState, StatePointer &destination, int tapeIndex);
    void moveMultipleTapes(TMTapeDirection direction, StatePointer &beginState, StatePointer &destination, const vector<int> tapeIndices);

    void immediateAddition(const string &variableName, string &binaryAddedValue, StatePointer &startingState,
                           StatePointer &destination, const string &variableContainingIndex="");

    void
    IntegerCompare(const string &variableName, string &binaryComparedValue, StatePointer &standardDestination,
                   int conditionalDestinationLineNumber, StatePointer beginState,
                   StatePointer conditionalEndState = nullptr, const string &indexContainingVariable="");

    void integerAssignment(const string &variableName, string &binaryAssignedValue, StatePointer &beginState,
                           StatePointer &destination, const string &variableContainingIndex= "");

    void doThingForEveryVoxelInCube(int x, int y, int z, StatePointer &beginState, StatePointer &destination,
                                    StatePointer thingStart, StatePointer thingEnd,
                                    const vector<int> &tapesToMove);

    void updateHistoryTape(int x, int y, int z, StatePointer &beginState, StatePointer &endState);
};




#endif //VOXELFUSION_TMGENERATOR_H
