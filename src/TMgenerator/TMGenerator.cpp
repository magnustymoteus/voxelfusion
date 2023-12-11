#include "TMGenerator.h"
using std::to_string, std::cout, std::cerr, std::endl, std::runtime_error;

TMGenerator::TMGenerator(set<string> &tapeAlphabet, map<TransitionDomain, TransitionImage> &transitions,
                         set<StatePointer> &states, bool readableStateNames) : tapeAlphabet(tapeAlphabet),
                                                                  transitions(transitions), states(states),
                                                                  postponedTransitionBuffer(list<PostponedTransition>()),
                                                                          readableStateNames(readableStateNames) {}

void TMGenerator::registerRegularNewline(StatePointer &state) {
    currentLineBeginState = state;
    currentLineNumber++;
    if(registerEndAsStartForNewLine){
        lineStartStates[currentLineNumber] = currentLineBeginState;
    }
}
TMTapeDirection TMGenerator::parseDirection(const shared_ptr<STNode> &root) {
    //<Direction> -> e.g. right
    if(root->label != "<Direction>"){
        throw runtime_error("This AST node does not contain a Direction");
    }
    char directionAlias = root->children[0]->token->lexeme[0] - 32; //lowercase to uppercase, please forgive me
    return (TMTapeDirection)directionAlias;
}

int TMGenerator::parseInteger(const shared_ptr<STNode> &root) {
    if(root->token->type != TokenType::Token_Integer){
        throw runtime_error("This AST node does not contain an Integer");
    }
    return std::stoi(root->token->lexeme);
}


void TMGenerator::assembleTasm(const shared_ptr<STNode> root) {
    StatePointer initializationState = make_shared<const State>(to_string(currentStateNumber), true);
    currentLineBeginState = make_shared<const State>(to_string(currentStateNumber), true);
    currentStateNumber++;
    states.insert(currentLineBeginState);
    lineStartStates[1] = currentLineBeginState;

    alphabetExplorer(root);
    tapeAlphabet.insert("VariableTapeStart");
    tapeAlphabet.insert("VariableTapeEnd");
    for (const string& ignoredSymbol: tapeAlphabet) {
        transitions.insert({
               TransitionDomain(initializationState, {ignoredSymbol, "B"}),
               TransitionImage(currentLineBeginState, {ignoredSymbol, "VariableTapeStart"}, {Stationary, Stationary})
       });
    }

    explorer(root);

    // for transitions that need to happen regardless of the symbols read, add them only when we know all possible symbols
    // TODO all symbols are now explored beforehand, remove useless code here
    // also, forward goto's!
    for(const PostponedTransition& transition: postponedTransitionBuffer){ //TODO clean up this code duplication
        StatePointer start = transition.startState == nullptr ? lineStartStates.at(transition.startLine) : transition.startState;
        StatePointer end = transition.endState == nullptr ? lineStartStates.at(transition.endLine) : transition.endState;
        if(transition.onlyTheseSymbols){
            for(auto& symbol: transition.leftOutSymbols){
                string replacedBy = transition.toWrite.empty() ? symbol : transition.toWrite;
                if(transition.tape == 0){
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        transitions.insert({
                               TransitionDomain(start, {symbol, ignoredSymbol}),
                               TransitionImage(end, {replacedBy, ignoredSymbol}, transition.directions)
                       });
                    }
                }else if (transition.tape == 1){
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        transitions.insert({
                               TransitionDomain(start, {ignoredSymbol, symbol}),
                               TransitionImage(end, {ignoredSymbol, replacedBy}, transition.directions)
                       });
                    }
                }

            }
        }
        else{
            for(const string& symbol: tapeAlphabet){
                if(transition.leftOutSymbols.find(symbol) != transition.leftOutSymbols.end()) continue;
                string replacedBy = transition.toWrite.empty() ? symbol : transition.toWrite;
                if(transition.tape == 0){
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        transitions.insert({
                                                   TransitionDomain(start, {symbol, ignoredSymbol}),
                                                   TransitionImage(end, {replacedBy, ignoredSymbol}, transition.directions)
                                           });
                    }
                }else if (transition.tape == 1){
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        transitions.insert({
                                                   TransitionDomain(start, {ignoredSymbol, symbol}),
                                                   TransitionImage(end, {ignoredSymbol, replacedBy}, transition.directions)
                                           });
                    }
                }
            }
        }
    }
}
void TMGenerator::explorer(const shared_ptr<STNode> &root) {
    string l = root->label;
    if(l == "<StatementList>"){
        if(root->children.size() == 2){
            explorer(root->children[1]);
        }
        explorer(root->children[0]);
    }else if(l == "<Statement>"){
        explorer(root->children[0]);
    }else{
        //cout << root->label << " " << root->token << endl;
        if(l == "<TapeMove>"){
            StatePointer destination = makeState(currentLineNumber +1);
            TMTapeDirection direction = parseDirection(root->children[1]);
            for(auto& symbol: tapeAlphabet){
                transitions.insert({
                                            TransitionDomain(currentLineBeginState, {symbol, "B"}),
                                            TransitionImage(destination, {symbol, "B"}, {direction, Stationary})
                                    });
            }
            registerRegularNewline(destination);
        }
        else if(l == "<TapeWrite>"){
            StatePointer destination = makeState(currentLineNumber +1);
            string symbolName = root->children[1]->token->lexeme;
            for(auto& symbol: tapeAlphabet){
                transitions.insert({
                                            TransitionDomain(currentLineBeginState, {symbol, "B"}),
                                            TransitionImage(destination, {symbolName, "B"}, {Stationary, Stationary})
                                    });
            }
            registerRegularNewline(destination);
        }
        else if(l == "<Jump>"){
            int destinationLineNumber = parseInteger(root->children[1]);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, destinationLineNumber);
            //next line prep
            StatePointer successor = makeState(currentLineNumber +1);
            registerRegularNewline(successor);
        }
        else if(l == "<ReadCondition>"){
            StatePointer standardDestination = makeState(currentLineNumber +1);
            string symbolName = root->children[3]->token->lexeme;
            postponedTransitionBuffer.emplace_back(currentLineBeginState, standardDestination, set<string>{symbolName});
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, conditionalDestinationLineNumber, set<string>{symbolName}, true);

            registerRegularNewline(standardDestination);
        }
        else if(l == "<ConditionalMove>"){
            TMTapeDirection direction = parseDirection(root->children.at(3));
            auto symbols = parseIdentifierList(root->children.at(1));
            postponedTransitionBuffer.emplace_back(currentLineBeginState, currentLineBeginState, symbols);
            std::next(postponedTransitionBuffer.end(), -1)->directions[0] = direction;
            StatePointer conditionalDestination = makeState(currentLineNumber +1);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, conditionalDestination, symbols, true);
            registerRegularNewline(conditionalDestination);
        }
        else if(l == "<Accept>"){
            StatePointer destination = makeState(currentLineNumber +1, true);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, destination);
            registerEndAsStartForNewLine = false;
            registerRegularNewline(destination);
            registerEndAsStartForNewLine = true;
        }
        else if(l == "<ImmediateSymbolValueAssignment>"){
            string variableName = root->children[3]->token->lexeme;
            for (const string& symbolToWrite: tapeAlphabet) {
                //search for the tape begin marker
                StatePointer goLeft = makeState();
                for (const string& ignoredSymbol: tapeAlphabet) {
                    if(ignoredSymbol == "VariableTapeStart") continue;
                    transitions.insert({
                           TransitionDomain(currentLineBeginState, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(goLeft, {symbolToWrite, ignoredSymbol}, {Stationary, Left})
                   });
                    transitions.insert({
                           TransitionDomain(goLeft, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(goLeft, {symbolToWrite, ignoredSymbol}, {Stationary, Left})
                   });
                }
                //move right until variable name or tape end found
                StatePointer goRight = makeState();
                transitions.insert({
                                           TransitionDomain(goLeft, {symbolToWrite, "VariableTapeStart"}),
                                           TransitionImage(goRight, {symbolToWrite, "VariableTapeStart"}, {Stationary, Right})
                                   });
                for (const string& ignoredSymbol: tapeAlphabet) {
                    if(ignoredSymbol == variableName || ignoredSymbol == "VariableTapeEnd") continue;
                    transitions.insert({
                           TransitionDomain(goRight, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(goRight, {symbolToWrite, ignoredSymbol}, {Stationary, Right})
                   });
                }

            }
        }
        else{
            cerr << "Instruction " << l << "is currently not supported by the compiler" << endl;
        }
    }
}

StatePointer TMGenerator::makeState(int beginStateOfThisLineNumber, bool accepting) {
    StatePointer newState;
    if(beginStateOfThisLineNumber == 0 || !readableStateNames){
        newState = make_shared<const State>(to_string(currentStateNumber), false, accepting ? State_Accepting : State_NonHalting);
        currentStateNumber++;
    }else if(accepting){
        newState = make_shared<const State>(string("Accept"), false, accepting ? State_Accepting : State_NonHalting);
    }
    else{
        newState = make_shared<const State>("\"Line " + to_string(beginStateOfThisLineNumber) + "\"", false, accepting ? State_Accepting : State_NonHalting);
    }
    states.insert(newState);
    return std::move(newState);
}

void TMGenerator::identifierListPartRecursiveParser(const shared_ptr<STNode> &root, set<string> &output) const {
    if(root->children.size() > 1){
        identifierListPartRecursiveParser(root->children.at(2), output);
    }
    output.insert(root->children[0]->token->lexeme);

}

set<string> TMGenerator::parseIdentifierList(const shared_ptr<STNode> &root) const{
    set<string> output;
    identifierListPartRecursiveParser(root->children.at(1), output);
    return output;
}

void TMGenerator::alphabetExplorer(const shared_ptr<STNode> &root) {
    if(root->token->type == TokenType::Token_Identifier){
        tapeAlphabet.insert(root->token->lexeme);
    }
    if(root->hasChildren()){
        for (auto& child : root->children) {
            alphabetExplorer(child);
        }
    }
}

PostponedTransition::PostponedTransition(const StatePointer& start, const StatePointer& end, const set<string>& leftOutSymbols, bool onlyTheseSymbols)
: startState(start), endState(end), startLine(0), endLine(0), leftOutSymbols(leftOutSymbols), onlyTheseSymbols(onlyTheseSymbols) {}

PostponedTransition::PostponedTransition(const StatePointer &startState, int endLine, const set<string>& leftOutSymbols, bool onlyTheseSymbols)
: startState(startState), endState(nullptr), startLine(0), endLine(endLine), leftOutSymbols(leftOutSymbols), onlyTheseSymbols(onlyTheseSymbols)  {}

PostponedTransition::PostponedTransition(int startLine, const StatePointer &endState, const set<string>& leftOutSymbols, bool onlyTheseSymbols)
: startState(nullptr), endState(endState), startLine(startLine), endLine(0), leftOutSymbols(leftOutSymbols), onlyTheseSymbols(onlyTheseSymbols)  {}

PostponedTransition::PostponedTransition(int startLine, int endLine, const set<string>& leftOutSymbols, bool onlyTheseSymbols)
: startState(nullptr), endState(nullptr), startLine(startLine), endLine(endLine), leftOutSymbols(leftOutSymbols), onlyTheseSymbols(onlyTheseSymbols)  {}
