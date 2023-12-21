#include <bitset>
#include "TMGenerator.h"
#define BINARY_VALUE_WIDTH 32
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
    StatePointer initializationState1 = make_shared<const State>(to_string(-2), true);
    states.insert(initializationState1);
    StatePointer initializationState2 = make_shared<const State>(to_string(-1), true);
    states.insert(initializationState2);
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


    for (const string& ignoredSymbol: tapeAlphabet) {
        //add start symbol
        transitions.insert({
               TransitionDomain(initializationState1, {ignoredSymbol, "B"}),
               TransitionImage(initializationState2, {ignoredSymbol, VariableTapeStart}, {Stationary, Right})
       });
        //add end symbol
        transitions.insert({
               TransitionDomain(initializationState2, {ignoredSymbol, "B"}),
               TransitionImage(currentLineBeginState, {ignoredSymbol, VariableTapeEnd}, {Stationary, Left})
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
            for (const string& symbolToWrite: tapeAlphabet) {
                //search for the tape begin marker
                StatePointer goLeft = makeState();
                for (const string& ignoredSymbol: tapeAlphabet) {
                    transitions.insert({
                           TransitionDomain(currentLineBeginState, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(goLeft, {symbolToWrite, ignoredSymbol}, {Stationary, Stationary})
                   });
                    if(ignoredSymbol != VariableTapeStart){
                        transitions.insert({
                               TransitionDomain(goLeft, {symbolToWrite, ignoredSymbol}),
                               TransitionImage(goLeft, {symbolToWrite, ignoredSymbol}, {Stationary, Left})
                       });
                    }

                }
                //move right until variable name or tape end found
                StatePointer goRight = makeState();
                transitions.insert({
                                           TransitionDomain(goLeft, {symbolToWrite, VariableTapeStart}),
                                           TransitionImage(goRight, {symbolToWrite, VariableTapeStart}, {Stationary, Right})
                                   });
                for (const string& ignoredSymbol: tapeAlphabet) {
                    if(ignoredSymbol == variableName || ignoredSymbol == VariableTapeEnd) continue;
                    transitions.insert({
                           TransitionDomain(goRight, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(goRight, {symbolToWrite, ignoredSymbol}, {Stationary, Right})
                   });
                }
                // option 1: variable name found: overwrite current value, whatever it is
                StatePointer writer = makeState();
                transitions.insert({
                                           TransitionDomain(goRight, {symbolToWrite, variableName}),
                                           TransitionImage(writer, {symbolToWrite, variableName}, {Stationary, Right})
                                   });
                for(const string& ignoredSymbol: tapeAlphabet){
                    if(ignoredSymbol == symbolToWrite) continue;
                    transitions.insert({
                           TransitionDomain(writer, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(writer, {symbolToWrite, symbolToWrite}, {Stationary, Stationary})
                   });
                }
                transitions.insert({
                       TransitionDomain(writer, {symbolToWrite, symbolToWrite}),
                       TransitionImage(destination, {symbolToWrite, symbolToWrite}, {Stationary, Right})
               });
                // option 2: tape end found: overwrite it and put a new tape end to the right
                StatePointer writeName = makeState();
                //write the name
                transitions.insert({
                       TransitionDomain(goRight, {symbolToWrite, VariableTapeEnd}),
                       TransitionImage(writeName, {symbolToWrite, variableName}, {Stationary, Right})
               });
                StatePointer writeValue = makeState();
                for(const string& ignoredSymbol: tapeAlphabet){
                    //write the value
                    transitions.insert({
                          TransitionDomain(writeName, {symbolToWrite, ignoredSymbol}),
                          TransitionImage(writeValue, {symbolToWrite, symbolToWrite}, {Stationary, Right})
                  });
                    if(ignoredSymbol == VariableTapeEnd) continue;
                    transitions.insert({
                           TransitionDomain(writeValue, {symbolToWrite, ignoredSymbol}),
                           TransitionImage(writeValue, {symbolToWrite, VariableTapeEnd}, {Stationary, Stationary})
                   });
                }
                transitions.insert({
                        TransitionDomain(writeValue, {symbolToWrite, VariableTapeEnd}),
                        TransitionImage(destination, {symbolToWrite, VariableTapeEnd}, {Stationary, Stationary})
                });
            }
            registerRegularNewline(destination);
        }
        else if(l == "<SymbolValueAssignment>"){
            string variableName = root->children[3]->token->lexeme;
            string variableValue = parseSymbolLiteral(root->children[1]);
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
            string variableValue = parseSymbolLiteral(root->children[3]);;
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

            //search for the tape begin marker
            StatePointer goLeft = makeState();
            postponedTransitionBuffer.emplace_back(currentLineBeginState, goLeft);
            postponedTransitionBuffer.emplace_back(goLeft, goLeft, set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            //move right until variable name or tape end found (stopping at tape end will fail any comparison so is better than going past the end)
            StatePointer goRight = makeState();
            postponedTransitionBuffer.emplace_back(goLeft, goRight, set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(goRight, goRight, set<string>{variableName, VariableTapeEnd});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

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

            //search for the tape begin marker
            StatePointer goLeft = makeState();
            postponedTransitionBuffer.emplace_back(currentLineBeginState, goLeft);
            postponedTransitionBuffer.emplace_back(goLeft, goLeft, set<string>{VariableTapeStart});
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Left;
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            //move right until variable name or tape end found (stopping at tape end will halt unexpectedly so is better than going past the end)
            StatePointer goRight = makeState();
            postponedTransitionBuffer.emplace_back(goLeft, goRight, set<string>{VariableTapeStart}, true);
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;
            postponedTransitionBuffer.emplace_back(goRight, goRight, set<string>{variableName, VariableTapeEnd});
            std::next(postponedTransitionBuffer.end(), -1)->tape = 1;
            std::next(postponedTransitionBuffer.end(), -1)->directions[1] = Right;

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
