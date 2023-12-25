#include <bitset>
#include "TMGenerator.h"
#include <algorithm>
#define BINARY_VALUE_WIDTH 6
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

string TMGenerator::parseSymbolLiteral(const shared_ptr<STNode> &root) {
    return root->children[1]->token->lexeme;
}

template<std::size_t N> // https://stackoverflow.com/questions/48556547/how-to-reverse-bits-in-a-bitset
void reverse(std::bitset<N> &b) {
    for(std::size_t i = 0; i < N/2; ++i) {
        bool t = b[i];
        b[i] = b[N-i-1];
        b[N-i-1] = t;
    }
}
string TMGenerator::IntegerAsBitString(int in, bool flipped) {
    auto bitset = std::bitset<BINARY_VALUE_WIDTH>(in);
    if(flipped) { // two's complement time
        bitset.flip();
        bitset = bitset.to_ulong() + 1;
    }
    reverse(bitset); //reverse to reduce necessary TM tape head travel e.g. for addition
    if(!flipped && bitset.test(0)) throw std::runtime_error("The MSB for a non-negative integer should be 0 but is 1, pick a larger BINARY_VALUE_WIDTH");
    return bitset.to_string();
}

void TMGenerator::assembleTasm(const shared_ptr<STNode> root) {
    StatePointer initializationState1 = make_shared<const State>(to_string(-3), true);
    states.insert(initializationState1);
    StatePointer initializationState2 = make_shared<const State>(to_string(-2), false);
    states.insert(initializationState2);
    StatePointer initializationState3 = make_shared<const State>(to_string(-1), false);
    states.insert(initializationState3);
    currentLineBeginState = make_shared<const State>(to_string(currentStateNumber), false);
    currentStateNumber++;
    states.insert(currentLineBeginState);
    lineStartStates[1] = currentLineBeginState;

    alphabetExplorer(root);
    //TODO variabeletape leegmaken voor te beginnen
    tapeAlphabet.insert(VariableTapeStart);
    tapeAlphabet.insert(VariableTapeEnd);
    tapeAlphabet.insert("0");
    tapeAlphabet.insert("1");


    vector<StatePointer> writeValueStates = {initializationState2};
    for (int i = 0; i < BINARY_VALUE_WIDTH - 1; ++i) {
        StatePointer writeValueState = make_shared<const State>("sysvar"+to_string(i), true);
        writeValueStates.push_back(writeValueState);
    }
    for (const string& ignoredSymbol: tapeAlphabet) {
        //add start symbol
        transitions.insert({
               TransitionDomain(initializationState1, {ignoredSymbol, "B", "B"}),
               TransitionImage(initializationState2, {ignoredSymbol, VariableTapeStart, "B"}, {Stationary, Right, Stationary})
       });
        // add system variable
        for (int i = 0; i < BINARY_VALUE_WIDTH -1; ++i) {
            StatePointer previous = writeValueStates[i];
            StatePointer newState = writeValueStates[i+1];
            transitions.insert({
                   TransitionDomain(previous, {ignoredSymbol, "B", "B"}),
                   TransitionImage(newState, {ignoredSymbol, "0", "B"}, {Stationary, Right, Stationary})
           });
        }
        transitions.insert({
               TransitionDomain(*std::next(writeValueStates.end(), -1), {ignoredSymbol, "B", "B"}),
               TransitionImage(initializationState3, {ignoredSymbol, "0", "B"}, {Stationary, Right, Stationary})
       });
        //add end symbol
        transitions.insert({
               TransitionDomain(initializationState3, {ignoredSymbol, "B", "B"}),
               TransitionImage(currentLineBeginState, {ignoredSymbol, VariableTapeEnd, "B"}, {Stationary, Left, Stationary})
       });
    }

    explorer(root);

    // for transitions that need to happen regardless of the symbols read, add them only when we know all possible symbols
    // also, forward goto's!
    for(const PostponedTransition& transition: postponedTransitionBuffer){
        StatePointer start = transition.startState == nullptr ? lineStartStates.at(transition.startLine) : transition.startState;
        StatePointer end = transition.endState == nullptr ? lineStartStates.at(transition.endLine) : transition.endState;
        set<string> relevantSymbols;
        if(transition.onlyTheseSymbols){
            relevantSymbols = transition.leftOutSymbols;
        }else{
            // all tape symbols except the left out symbols
            std::set_difference(tapeAlphabet.begin(), tapeAlphabet.end(), transition.leftOutSymbols.begin(),
                                transition.leftOutSymbols.end(), std::inserter(relevantSymbols, relevantSymbols.end()));
        }
        for(const string& ignoredSymbol: tapeAlphabet){
            for(const string& ignoredSymbol2: tapeAlphabet){
                for (const string& symbol: relevantSymbols) {
                    string replacedBy = transition.toWrite.empty() ? symbol : transition.toWrite;
                    vector<string> replacedSymbols{ignoredSymbol, ignoredSymbol2};
                    vector<string> replacementSymbols{ignoredSymbol, ignoredSymbol2};
                    replacedSymbols.insert(next(replacedSymbols.begin(), transition.tape), symbol);
                    replacementSymbols.insert(next(replacementSymbols.begin(), transition.tape), replacedBy);
                    transitions.insert({
                           TransitionDomain(start, replacedSymbols),
                           TransitionImage(end, replacementSymbols, transition.directions)
                   });
                }
            }
        }
    }
    cout << "Generated a Finite Control with " << states.size() << " states and " << transitions.size() << " transitions" << endl;
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
            postponedTransitionBuffer.emplace_back(currentLineBeginState, destination);
            std::next(postponedTransitionBuffer.end(), -1)->directions[0] = direction;
            registerRegularNewline(destination);
        }
        else if(l == "<TapeWrite>"){
            StatePointer destination = makeState(currentLineNumber +1);
            string symbolName = parseSymbolLiteral(root->children[1]);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, destination);
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = symbolName;
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
            string symbolName = parseSymbolLiteral(root->children[3]);
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
            StatePointer destination = makeState(currentLineNumber +1);
            for (const string& ignoredSymbol2: tapeAlphabet) {
                for (const string& symbolToWrite: tapeAlphabet) {
                    //search for the tape begin marker
                    StatePointer goLeft = makeState();
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        transitions.insert({
                                                   TransitionDomain(currentLineBeginState, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                   TransitionImage(goLeft, {symbolToWrite, ignoredSymbol, ignoredSymbol2}, {Stationary, Stationary, Stationary})
                                           });
                        if(ignoredSymbol != VariableTapeStart){
                            transitions.insert({
                                                       TransitionDomain(goLeft, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                       TransitionImage(goLeft, {symbolToWrite, ignoredSymbol, ignoredSymbol2}, {Stationary, Left, Stationary})
                                               });
                        }

                    }
                    //move right until variable name or tape end found
                    StatePointer goRight = makeState();
                    transitions.insert({
                                               TransitionDomain(goLeft, {symbolToWrite, VariableTapeStart, ignoredSymbol2}),
                                               TransitionImage(goRight, {symbolToWrite, VariableTapeStart, ignoredSymbol2}, {Stationary, Right, Stationary})
                                       });
                    for (const string& ignoredSymbol: tapeAlphabet) {
                        if(ignoredSymbol == variableName || ignoredSymbol == VariableTapeEnd) continue;
                        transitions.insert({
                                                   TransitionDomain(goRight, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                   TransitionImage(goRight, {symbolToWrite, ignoredSymbol, ignoredSymbol2}, {Stationary, Right, Stationary})
                                           });
                    }
                    // option 1: variable name found: overwrite current value, whatever it is
                    StatePointer writer = makeState();
                    transitions.insert({
                                               TransitionDomain(goRight, {symbolToWrite, variableName, ignoredSymbol2}),
                                               TransitionImage(writer, {symbolToWrite, variableName, ignoredSymbol2}, {Stationary, Right, Stationary})
                                       });
                    for(const string& ignoredSymbol: tapeAlphabet){
                        if(ignoredSymbol == symbolToWrite) continue;
                        transitions.insert({
                                                   TransitionDomain(writer, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                   TransitionImage(writer, {symbolToWrite, symbolToWrite, ignoredSymbol2}, {Stationary, Stationary, Stationary})
                                           });
                    }
                    transitions.insert({
                                               TransitionDomain(writer, {symbolToWrite, symbolToWrite, ignoredSymbol2}),
                                               TransitionImage(destination, {symbolToWrite, symbolToWrite, ignoredSymbol2}, {Stationary, Right, Stationary})
                                       });
                    // option 2: tape end found: overwrite it and put a new tape end to the right
                    StatePointer writeName = makeState();
                    //write the name
                    transitions.insert({
                                               TransitionDomain(goRight, {symbolToWrite, VariableTapeEnd, ignoredSymbol2}),
                                               TransitionImage(writeName, {symbolToWrite, variableName, ignoredSymbol2}, {Stationary, Right, Stationary})
                                       });
                    StatePointer writeValue = makeState();
                    for(const string& ignoredSymbol: tapeAlphabet){
                        //write the value
                        transitions.insert({
                                                   TransitionDomain(writeName, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                   TransitionImage(writeValue, {symbolToWrite, symbolToWrite, ignoredSymbol2}, {Stationary, Right, Stationary})
                                           });
                        if(ignoredSymbol == VariableTapeEnd) continue;
                        transitions.insert({
                                                   TransitionDomain(writeValue, {symbolToWrite, ignoredSymbol, ignoredSymbol2}),
                                                   TransitionImage(writeValue, {symbolToWrite, VariableTapeEnd, ignoredSymbol2}, {Stationary, Stationary, Stationary})
                                           });
                    }
                    transitions.insert({
                                               TransitionDomain(writeValue, {symbolToWrite, VariableTapeEnd, ignoredSymbol2}),
                                               TransitionImage(destination, {symbolToWrite, VariableTapeEnd, ignoredSymbol2}, {Stationary, Stationary, Stationary})
                                       });
                }
            }
            registerRegularNewline(destination);
        }
        else if(l == "<SymbolValueAssignment>"){
            string variableName = root->children[3]->token->lexeme;
            string variableValue = parseSymbolLiteral(root->children[1]);
            StatePointer destination = makeState(currentLineNumber +1);
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, variableName);
            // option 1: variable name found: overwrite current value, whatever it is
                StatePointer writer = makeState();
                postponedTransitionBuffer.emplace_back(goRight, writer, set<string>{variableName}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                postponedTransitionBuffer.emplace_back(writer, writer, set<string>{variableValue});
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = variableValue;
                postponedTransitionBuffer.emplace_back(writer, destination);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            // option 2: tape end found: overwrite it and put a new tape end to the right
                StatePointer writeName = makeState();
                //write the name
                postponedTransitionBuffer.emplace_back(goRight, writeName, set<string>{VariableTapeEnd}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = variableName;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                StatePointer writeValue = makeState();
                postponedTransitionBuffer.emplace_back(writeName, writeValue);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = variableValue;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                postponedTransitionBuffer.emplace_back(writeValue, writeValue, set<string>{VariableTapeEnd});
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = VariableTapeEnd;
                postponedTransitionBuffer.emplace_back(writeValue, destination, set<string>{VariableTapeEnd}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            registerRegularNewline(destination);
        }
        else if(l == "<SymbolVariableCondition>"){
            string variableName = root->children[5]->token->lexeme;
            string variableValue = parseSymbolLiteral(root->children[3]);
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            StatePointer destination = makeState(currentLineNumber +1);
            //search for the tape begin marker
            StatePointer goLeft = makeState();
            postponedTransitionBuffer.emplace_back(currentLineBeginState, goLeft);
            postponedTransitionBuffer.emplace_back(goLeft, goLeft, set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            //move right until variable name or tape end found
            StatePointer goRight = makeState();
            postponedTransitionBuffer.emplace_back(goLeft, goRight, set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(goRight, goRight, set<string>{variableName, VariableTapeEnd});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            // move to look at the value
            StatePointer observe = makeState();
            postponedTransitionBuffer.emplace_back(goRight, observe);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            //regular condition logic
            StatePointer standardDestination = makeState(currentLineNumber +1);
            postponedTransitionBuffer.emplace_back(observe, standardDestination, set<string>{variableValue});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            postponedTransitionBuffer.emplace_back(observe, conditionalDestinationLineNumber, set<string>{variableValue}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;

            registerRegularNewline(standardDestination);

        }else if(l == "<IntegerValueAssignment>"){
            string variableName = root->children[3]->token->lexeme;
            int assignedValue = parseInteger(root->children[1]);
            std::string binaryAssignedValue = IntegerAsBitString(assignedValue);
            StatePointer destination = makeState(currentLineNumber +1);
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, variableName);

            // option 1: variable name found: overwrite current value, whatever it is
                StatePointer writer1 = makeState();
                postponedTransitionBuffer.emplace_back(goRight, writer1, set<string>{variableName}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                vector<StatePointer> writeValueStates1 = { writer1};
                for (char c : binaryAssignedValue) {
                    writeValueStates1.emplace_back(makeState());
                    auto last = std::next(writeValueStates1.end(), -1);
                    auto penultimate = std::next(last, -1);
                    postponedTransitionBuffer.emplace_back(*penultimate, *last);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = c;
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                }
                postponedTransitionBuffer.emplace_back(*std::next(writeValueStates1.end(), -1), destination);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Stationary;
            // option 2: tape end found: overwrite it and put a new tape end to the right
                StatePointer writeName = makeState();
                //write the name
                postponedTransitionBuffer.emplace_back(goRight, writeName, set<string>{VariableTapeEnd}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = variableName;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

                vector<StatePointer> writeValueStates2 = {writeName};
                for (char c : binaryAssignedValue) {
                    writeValueStates2.emplace_back(makeState());
                    auto last = std::next(writeValueStates2.end(), -1);
                    auto penultimate = std::next(last, -1);
                    postponedTransitionBuffer.emplace_back(*penultimate, *last);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = c;
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                }
                StatePointer writeTapeEnd = makeState();
                postponedTransitionBuffer.emplace_back(*std::next(writeValueStates2.end(), -1), writeTapeEnd, set<string>{VariableTapeEnd});
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->toWrite = VariableTapeEnd;
                postponedTransitionBuffer.emplace_back(writeTapeEnd, destination, set<string>{VariableTapeEnd}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            registerRegularNewline(destination);
        }
        else if(l == "<IntegerVariableCondition>"){
            string variableName = root->children[5]->token->lexeme;
            int comparedValue = parseInteger(root->children[3]);
            std::string binaryComparedValue = IntegerAsBitString(comparedValue);
            StatePointer standardDestination = makeState(currentLineNumber +1);
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, variableName);

            //bitwise comparison
            StatePointer reader1 = makeState();
            postponedTransitionBuffer.emplace_back(goRight, reader1, set<string>{variableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            vector<StatePointer> readerStates = {reader1};
            for (char c : binaryComparedValue) {
                readerStates.emplace_back(makeState());
                auto last = std::next(readerStates.end(), -1);
                auto penultimate = std::next(last, -1);
                postponedTransitionBuffer.emplace_back(*penultimate, *last, set<string>{string(1, c)}, true);
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                postponedTransitionBuffer.emplace_back(*penultimate, standardDestination, set<string>{string(1, c)});
                std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Stationary;
            }
            postponedTransitionBuffer.emplace_back(*std::next(readerStates.end(), -1), conditionalDestinationLineNumber);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            registerRegularNewline(standardDestination);
        }
        else if(l == "<ImmediateAddition>" || l == "<ImmediateSubtraction>"){
            string variableName = root->children[3]->token->lexeme;
            int addedValue = parseInteger(root->children[1]);
            std::string binaryAddedValue = IntegerAsBitString(addedValue, l == "<ImmediateSubtraction>");
            StatePointer destination = makeState(currentLineNumber +1);
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, variableName);

            //start adding
            StatePointer writer1 = makeState();
            postponedTransitionBuffer.emplace_back(goRight, writer1, set<string>{variableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            vector<StatePointer> writeValueStates = {writer1, makeState()}; // first carry state cannot be reached here but makes the loop below easier
            for (char c : binaryAddedValue) {
                StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
                StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
                StatePointer newNormalState = makeState();
                StatePointer newCarryState = makeState();
                writeValueStates.push_back(newNormalState);
                writeValueStates.push_back(newCarryState);
                if(c == '0'){
                    postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, set<string>{"0"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, set<string>{"1"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldCarryState, newNormalState, set<string>{"0"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "1";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, set<string>{"1"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                }else if(c == '1'){
                    postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, set<string>{"0"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "1";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldNormalState, newCarryState, set<string>{"1"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, set<string>{"0"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                    postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, set<string>{"1"}, true);
                    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
                    std::next(postponedTransitionBuffer.end(), -1)->toWrite = "1";
                    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
                }
            }
            StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
            StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
            postponedTransitionBuffer.emplace_back(oldNormalState, destination);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Stationary;
            postponedTransitionBuffer.emplace_back(oldCarryState, destination);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Stationary;
            registerRegularNewline(destination);

        }
        else if(l == "<BinaryAddition>"){
            //seek second variable and copy to third tape
            string assignedVariableName = root->children[3]->token->lexeme;
            string readVariableName = root->children[1]->token->lexeme;
            StatePointer destination = makeState(currentLineNumber +1);
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, readVariableName);
            StatePointer moveToValue = makeState();
            postponedTransitionBuffer.emplace_back(goRight, moveToValue, std::set<string>{readVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            StatePointer doneCopying = copyIntegerToThirdTape(moveToValue, readVariableName);
            // seek first variable
            StatePointer doneMoving = MoveToVariableMarker(doneCopying, assignedVariableName);

            //move to look at the value
            // start adding
            vector<StatePointer> writeValueStates = {makeState(), makeState()}; // first carry state cannot be reached here but makes the loop below easier
            postponedTransitionBuffer.emplace_back(doneMoving, writeValueStates[0], set<string>{assignedVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            addThirdToSecond(writeValueStates);
            //move back to start of third tape and erase
            StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
            StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
            StatePointer eraser = makeState();

            postponedTransitionBuffer.emplace_back(oldNormalState, eraser);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(oldCarryState, eraser);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;

            postponedTransitionBuffer.emplace_back(eraser, eraser, set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";

            postponedTransitionBuffer.emplace_back(eraser, destination, set<string>{"B"}, true);

            registerRegularNewline(destination);
        }
        else if(l == "<BinaryVariableCondition>"){
            string leftVariableName = root->children[5]->token->lexeme;
            string rightVariableName = root->children[3]->token->lexeme;
            StatePointer standardDestination = makeState(currentLineNumber +1);
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);

            // seek second variable and copy it to third tape
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, rightVariableName);
            StatePointer moveToValue = makeState();
            postponedTransitionBuffer.emplace_back(goRight, moveToValue, std::set<string>{rightVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            // if integer
            StatePointer doneCopying = copyIntegerToThirdTape(moveToValue, rightVariableName);
            StatePointer goRight2 = MoveToVariableMarker(doneCopying, leftVariableName);
            vector<StatePointer> checkValueStates = {makeState()};
            postponedTransitionBuffer.emplace_back(goRight2, checkValueStates[0], set<string>{leftVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            StatePointer falseEraser = makeState();

            for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
                StatePointer oldState = *std::next(checkValueStates.end(), -1);
                StatePointer newState = makeState();
                checkValueStates.push_back(newState);
                for(const string& ignoredSymbol: tapeAlphabet){
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "0", "0"}),
                                               TransitionImage(newState, {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "1", "1"}),
                                               TransitionImage(newState, {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "1", "0"}),
                                               TransitionImage(falseEraser, {ignoredSymbol, "1", "0"}, {Stationary, Stationary, Stationary})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "0", "1"}),
                                               TransitionImage(falseEraser, {ignoredSymbol, "0", "1"}, {Stationary, Stationary, Stationary})
                                       });

                }
            }
            // erase if false
            postponedTransitionBuffer.emplace_back(falseEraser, falseEraser, set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            StatePointer falseEraser2 = makeState();
            postponedTransitionBuffer.emplace_back(falseEraser, falseEraser2, set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            StatePointer falseEraser3 = makeState();
            postponedTransitionBuffer.emplace_back(falseEraser2, falseEraser3, set<string>{"0", "1", "B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            postponedTransitionBuffer.emplace_back(falseEraser3, falseEraser3, set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            postponedTransitionBuffer.emplace_back(falseEraser3, standardDestination, set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Stationary;

            //erase if true
            StatePointer trueEraser = makeState();
            postponedTransitionBuffer.emplace_back(*std::next(checkValueStates.end(), -1), trueEraser);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(trueEraser, trueEraser, set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            postponedTransitionBuffer.emplace_back(trueEraser, conditionalDestinationLineNumber, set<string>{"B"}, true);

            //if symbol
            //copy to third tape
            StatePointer doneCopying2 = makeState();
            for(const string& ignoredSymbol: tapeAlphabet){
                for(const string& copiedSymbol: tapeAlphabet){
                    if(copiedSymbol == "0" || copiedSymbol == "1") continue;
                    transitions.insert({
                           TransitionDomain(moveToValue, {ignoredSymbol, copiedSymbol, "B"}),
                           TransitionImage(doneCopying2, {ignoredSymbol, copiedSymbol, copiedSymbol}, {Stationary, Right, Stationary})
                   });
                }
            }
            //move to the value
            StatePointer goRight3 = MoveToVariableMarker(doneCopying2, leftVariableName);
            StatePointer checkSingleValue = makeState();
            postponedTransitionBuffer.emplace_back(goRight3, checkSingleValue, set<string>{leftVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            StatePointer symbolTrueIntermediate = makeState();
            postponedTransitionBuffer.emplace_back(symbolTrueIntermediate, conditionalDestinationLineNumber);
            for(const string& ignoredSymbol: tapeAlphabet){
                for(const string& comparedSymbol1: tapeAlphabet){
                    if(comparedSymbol1 == "0" || comparedSymbol1 == "1") continue;
                    transitions.insert({
                           TransitionDomain(checkSingleValue, {ignoredSymbol, comparedSymbol1, comparedSymbol1}),
                           TransitionImage(symbolTrueIntermediate, {ignoredSymbol, comparedSymbol1, "B"}, {Stationary, Stationary, Stationary})
                   });
                    for(const string& comparedSymbol2: tapeAlphabet){
                        if(comparedSymbol2 == "0" || comparedSymbol2 == "1") continue;
                        if(comparedSymbol1 != comparedSymbol2){
                            transitions.insert({
                                   TransitionDomain(checkSingleValue, {ignoredSymbol, comparedSymbol1, comparedSymbol2}),
                                   TransitionImage(standardDestination, {ignoredSymbol, comparedSymbol1, "B"}, {Stationary, Stationary, Stationary})
                           });

                        }
                    }
                }
            }
            registerRegularNewline(standardDestination);
        }
        else if(l == "<BinaryMultiplication>"){
            string assignedVariableName = root->children[3]->token->lexeme;
            string readVariableName = root->children[1]->token->lexeme;
            StatePointer destination = makeState(currentLineNumber +1);
            // copy the multiplier to third tape
            StatePointer goRight = MoveToVariableMarker(currentLineBeginState, readVariableName);
            StatePointer moveToValue = makeState();
            postponedTransitionBuffer.emplace_back(goRight, moveToValue, std::set<string>{readVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            StatePointer doneCopying = copyIntegerToThirdTape(moveToValue, readVariableName);
            // copy the multiplier to sysvar
            StatePointer moveToVTB = makeState();
            postponedTransitionBuffer.emplace_back(doneCopying, moveToVTB, std::set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            postponedTransitionBuffer.emplace_back(moveToVTB, moveToVTB, std::set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;

            vector<StatePointer> writeValueStates = {makeState()};
            postponedTransitionBuffer.emplace_back(moveToVTB, writeValueStates[0], set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
                StatePointer oldState = *std::next(writeValueStates.end(), -1);
                StatePointer newState = makeState();
                writeValueStates.push_back(newState);
                for(const string& ignoredSymbol: tapeAlphabet){
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "0", "0"}),
                                               TransitionImage(newState, {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "1", "1"}),
                                               TransitionImage(newState, {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "1", "0"}),
                                               TransitionImage(newState, {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {ignoredSymbol, "0", "1"}),
                                               TransitionImage(newState, {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
                                       });

                }
            }

            StatePointer eraser = makeState();
            postponedTransitionBuffer.emplace_back(*std::next(writeValueStates.end(), -1), eraser);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            // erase multiplier from sysvar
            postponedTransitionBuffer.emplace_back(eraser, eraser, set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            // put multiplicand on third tape
            StatePointer goRight2 = MoveToVariableMarker(eraser, assignedVariableName);
            StatePointer moveToValue2 = makeState();
            postponedTransitionBuffer.emplace_back(goRight2, moveToValue2, std::set<string>{assignedVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            StatePointer doneCopying2 = copyIntegerToThirdTape(moveToValue2, assignedVariableName);
            // erase multiplicand from second tape to start from 0 properly
            StatePointer eraseMultiplicand = makeState();
            postponedTransitionBuffer.emplace_back(doneCopying2, eraseMultiplicand, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
            postponedTransitionBuffer.emplace_back(eraseMultiplicand, eraseMultiplicand, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
            //check counter != 0
            StatePointer moveToVTB2 = makeState();
            postponedTransitionBuffer.emplace_back(eraseMultiplicand, moveToVTB2, std::set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            postponedTransitionBuffer.emplace_back(moveToVTB2, moveToVTB2, std::set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            StatePointer checkIsNotZero = makeState();
            StatePointer prepareCounterDecrement = makeState();
            StatePointer multiplicationDone = makeState();
            postponedTransitionBuffer.emplace_back(moveToVTB2, checkIsNotZero, std::set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, checkIsNotZero, std::set<string>{"0"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, prepareCounterDecrement, std::set<string>{"1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, multiplicationDone, std::set<string>{VariableTapeStart, "0", "1"});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

            // decrement counter
            // move to counter start
            postponedTransitionBuffer.emplace_back(prepareCounterDecrement, prepareCounterDecrement, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            StatePointer counterDecrement = makeState();
            postponedTransitionBuffer.emplace_back(prepareCounterDecrement, counterDecrement, std::set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            StatePointer counterDecrementBorrowing = makeState();
            StatePointer doneDecrementing = makeState();
            postponedTransitionBuffer.emplace_back(counterDecrement, doneDecrementing, std::set<string>{"1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";
            postponedTransitionBuffer.emplace_back(counterDecrement, counterDecrementBorrowing, std::set<string>{"0"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "1";
            postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, counterDecrementBorrowing, std::set<string>{"0"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "1";
            postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, doneDecrementing, std::set<string>{"1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "0";

            StatePointer moveBackToMultiplicand = makeState();
            postponedTransitionBuffer.emplace_back(doneDecrementing, moveBackToMultiplicand, std::set<string>{assignedVariableName});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(moveBackToMultiplicand, moveBackToMultiplicand, std::set<string>{assignedVariableName});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            StatePointer moveBackToMultiplicandValue = makeState();
            postponedTransitionBuffer.emplace_back(moveBackToMultiplicand, moveBackToMultiplicandValue, std::set<string>{assignedVariableName}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;


            // add once
            vector<StatePointer> writeValueStates2 = {moveBackToMultiplicandValue, makeState()};
            addThirdToSecond(writeValueStates2);
            StatePointer oldNormalState = *std::next(writeValueStates2.end(), -2);
            StatePointer oldCarryState = *std::next(writeValueStates2.end(), -1);
            //Tape head on third tape back to start
            StatePointer ThirdTapeBackToStart1 = makeState();
            StatePointer ThirdTapeBackToStart2 = makeState();
            postponedTransitionBuffer.emplace_back(oldNormalState, ThirdTapeBackToStart1, std::set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(oldCarryState, ThirdTapeBackToStart1, std::set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart1, ThirdTapeBackToStart2, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart2, ThirdTapeBackToStart2, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;

            // go back to check counter
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart2, moveToVTB2, std::set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Right;

            // if multiplication done, erase third tape
            postponedTransitionBuffer.emplace_back(multiplicationDone, multiplicationDone, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Right;
            StatePointer eraser2 = makeState();
            postponedTransitionBuffer.emplace_back(multiplicationDone, eraser2, std::set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            postponedTransitionBuffer.emplace_back(eraser2, eraser2, std::set<string>{"0", "1"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->toWrite = "B";
            postponedTransitionBuffer.emplace_back(eraser2, destination, std::set<string>{"B"}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 2;
            std::next(postponedTransitionBuffer.end(), -1)->directions[2] = Left;



            registerRegularNewline(destination);
        }
        else{
            cerr << "Instruction " << l << "is currently not supported by the compiler" << endl;
        }
    }
}

void TMGenerator::addThirdToSecond(vector<StatePointer> &writeValueStates) {
    for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
        StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
        StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
        StatePointer newNormalState = makeState();
        StatePointer newCarryState = makeState();
        writeValueStates.push_back(newNormalState);
        writeValueStates.push_back(newCarryState);
        for(const string& ignoredSymbol: tapeAlphabet){
            //no carry
            transitions.insert({
                                       TransitionDomain(oldNormalState, {ignoredSymbol, "0", "0"}),
                                       TransitionImage(newNormalState, {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {ignoredSymbol, "1", "0"}),
                                       TransitionImage(newNormalState, {ignoredSymbol, "1", "0"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {ignoredSymbol, "0", "1"}),
                                       TransitionImage(newNormalState, {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {ignoredSymbol, "1", "1"}),
                                       TransitionImage(newCarryState, {ignoredSymbol, "0", "1"}, {Stationary, Right, Right})
                               });
            //yes carry
            transitions.insert({
                                       TransitionDomain(oldCarryState, {ignoredSymbol, "0", "0"}),
                                       TransitionImage(newNormalState, {ignoredSymbol, "1", "0"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {ignoredSymbol, "1", "0"}),
                                       TransitionImage(newCarryState, {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {ignoredSymbol, "0", "1"}),
                                       TransitionImage(newCarryState, {ignoredSymbol, "0", "1"}, {Stationary, Right, Right})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {ignoredSymbol, "1", "1"}),
                                       TransitionImage(newCarryState, {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
                               });

        }
    }
}

// Assumes the tape head is already moved to the variable marker
StatePointer TMGenerator::copyIntegerToThirdTape(StatePointer startState, const string &variableName) {
    //copy to third tape and return to start
    std::vector<StatePointer> copyStates = {startState};

    StatePointer doneCopying = makeState();

    for (int i = 0; i < BINARY_VALUE_WIDTH - 1; ++i) {
        copyStates.push_back(makeState());
        for(const string& ignoredSymbol: tapeAlphabet){
            transitions.insert({
                   TransitionDomain(copyStates[i], {ignoredSymbol, "0", "B"}),
                   TransitionImage(copyStates[i+1], {ignoredSymbol, "0", "0"}, {Stationary, Right, Right})
           });
            transitions.insert({
                   TransitionDomain(copyStates[i], {ignoredSymbol, "1", "B"}),
                   TransitionImage(copyStates[i+1], {ignoredSymbol, "1", "1"}, {Stationary, Right, Right})
           });
            //prepare return
            if(i == 0){
                transitions.insert({
                       TransitionDomain(copyStates[i+1], {ignoredSymbol, "0", "0"}),
                       TransitionImage(doneCopying, {ignoredSymbol, "0", "0"}, {Stationary, Left, Left})
               });
                transitions.insert({
                       TransitionDomain(copyStates[i+1], {ignoredSymbol, "1", "1"}),
                       TransitionImage(doneCopying, {ignoredSymbol, "1", "1"}, {Stationary, Left, Left})
               });
            }else{
                transitions.insert({
                       TransitionDomain(copyStates[i+1], {ignoredSymbol, "0", "0"}),
                       TransitionImage(copyStates[i], {ignoredSymbol, "0", "0"}, {Stationary, Left, Left})
               });
                transitions.insert({
                       TransitionDomain(copyStates[i+1], {ignoredSymbol, "1", "1"}),
                       TransitionImage(copyStates[i], {ignoredSymbol, "1", "1"}, {Stationary, Left, Left})
               });
            }

        }
    }
    //the last one is special
    for(const string& ignoredSymbol: tapeAlphabet){
        transitions.insert({
                                   TransitionDomain(copyStates[BINARY_VALUE_WIDTH - 1], {ignoredSymbol, "0", "B"}),
                                   TransitionImage(copyStates[BINARY_VALUE_WIDTH - 1], {ignoredSymbol, "0", "0"}, {Stationary, Stationary, Stationary})
                           });
        transitions.insert({
                                   TransitionDomain(copyStates[BINARY_VALUE_WIDTH - 1], {ignoredSymbol, "1", "B"}),
                                   TransitionImage(copyStates[BINARY_VALUE_WIDTH - 1], {ignoredSymbol, "1", "1"}, {Stationary, Stationary, Stationary})
                           });

    }
    return doneCopying;
}

StatePointer TMGenerator::MoveToVariableMarker(StatePointer startState, const string &variableName) {
    //search for the tape begin marker
    StatePointer goLeft = makeState();
    postponedTransitionBuffer.emplace_back(startState, goLeft);
    postponedTransitionBuffer.emplace_back(goLeft, goLeft, std::set<string>{VariableTapeStart});
    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
    //move right until variable name or tape end found (stopping at tape end will halt unexpectedly so is better than going past the end)
    StatePointer goRight = makeState();
    postponedTransitionBuffer.emplace_back(goLeft, goRight, std::set<string>{VariableTapeStart}, true);
    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
    postponedTransitionBuffer.emplace_back(goRight, goRight, std::set<string>{variableName, VariableTapeEnd});
    std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
    std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
    return goRight;
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
    return newState;
}

void TMGenerator::identifierListPartRecursiveParser(const shared_ptr<STNode> &root, set<string> &output) {
    if(root->children.size() > 1){
        identifierListPartRecursiveParser(root->children.at(2), output);
    }
    output.insert(parseSymbolLiteral(root->children[0]));

}

set<string> TMGenerator::parseIdentifierList(const shared_ptr<STNode> &root) {
    set<string> output;
    identifierListPartRecursiveParser(root->children.at(1), output);
    return output;
}

void TMGenerator::alphabetExplorer(const shared_ptr<STNode> &root) {
    if(root->token != nullptr && root->token->type == TokenType::Token_Identifier){
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
