#include <bitset>
#include "TMGenerator.h"
#include <algorithm>

using std::to_string, std::cout, std::cerr, std::endl, std::runtime_error;

TMGenerator::TMGenerator(set<string> &tapeAlphabet, map<TransitionDomain, TransitionImage> &transitions,
                         set<StatePointer> &states, bool readableStateNames) : tapeAlphabet(tapeAlphabet),
                                                                  transitions(transitions), states(states),
                                                                  postponedTransitionBuffer(list<PostponedTransition>()),
                                                                          readableStateNames(readableStateNames) {}

void TMGenerator::registerRegularNewline(StatePointer &state) {
    currentLineBeginState = state;
    currentLineNumber++;
    lineStartStates[currentLineNumber] = currentLineBeginState;
}
StatePointer TMGenerator::getNextLineStartState() {
    currentLineBeginState = makeState(currentLineNumber +1);
    currentLineNumber++;
    lineStartStates[currentLineNumber] = currentLineBeginState;

    return currentLineBeginState;
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
    tapeAlphabet.insert("Left");
    tapeAlphabet.insert("Right");
    tapeAlphabet.insert("Up");
    tapeAlphabet.insert("Down");
    tapeAlphabet.insert("Front");
    tapeAlphabet.insert("Back");
    tapeAlphabet.insert("Xcounter");
    tapeAlphabet.insert("Ycounter");
    tapeAlphabet.insert("Zcounter");


    vector<StatePointer> writeValueStates = {initializationState2};
    for (int i = 0; i < BINARY_VALUE_WIDTH - 1; ++i) {
        StatePointer writeValueState = make_shared<const State>("sysvar"+to_string(i), true);
        writeValueStates.push_back(writeValueState);
    }
    //add start symbol
    transitions.insert({
           TransitionDomain(initializationState1, {SYMBOL_ANY, "B", "B", SYMBOL_ANY}),
           TransitionImage(initializationState2, {SYMBOL_ANY, VariableTapeStart, "B", SYMBOL_ANY}, {Stationary, Right, Stationary, Stationary})
   });
    // add system variable
    for (int i = 0; i < BINARY_VALUE_WIDTH -1; ++i) {
        StatePointer previous = writeValueStates[i];
        StatePointer newState = writeValueStates[i+1];
        transitions.insert({
               TransitionDomain(previous, {SYMBOL_ANY, "B", "B", SYMBOL_ANY}),
               TransitionImage(newState, {SYMBOL_ANY, "0", "B", SYMBOL_ANY}, {Stationary, Right, Stationary, Stationary})
       });
    }
    transitions.insert({
           TransitionDomain(*std::next(writeValueStates.end(), -1), {SYMBOL_ANY, "B", "B", SYMBOL_ANY}),
           TransitionImage(initializationState3, {SYMBOL_ANY, "0", "B", SYMBOL_ANY}, {Stationary, Right, Stationary, Stationary})
   });
    //add end symbol
    transitions.insert({
           TransitionDomain(initializationState3, {SYMBOL_ANY, "B", "B", SYMBOL_ANY}),
           TransitionImage(currentLineBeginState, {SYMBOL_ANY, VariableTapeEnd, "B", SYMBOL_ANY}, {Stationary, Left, Stationary, Stationary})
   });

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
        for (const string& symbol: relevantSymbols) {
            string replacedBy = transition.toWrite.empty() ? symbol : transition.toWrite;
            vector<string> replacedSymbols{SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY};
            vector<string> replacementSymbols{SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY};
            replacedSymbols.insert(next(replacedSymbols.begin(), transition.tape), symbol);
            replacementSymbols.insert(next(replacementSymbols.begin(), transition.tape), replacedBy);
            transitions.insert({
                                       TransitionDomain(start, replacedSymbols),
                                       TransitionImage(end, replacementSymbols, transition.directions)
                               });
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
        if(l == "<TapeMove>"){
            StatePointer first = currentLineBeginState;
            StatePointer destination = getNextLineStartState();
            TMTapeDirection direction = parseDirection(root->children[1]);
            tapeMove(direction, first, destination, 0);
        }
        else if(l == "<TapeWrite>"){
            StatePointer first = currentLineBeginState;
            StatePointer destination = getNextLineStartState();
            string symbolName = parseSymbolLiteral(root->children[1]);
            postponedTransitionBuffer.emplace_back(first, destination);
            postponedTransitionBuffer.back().toWrite = symbolName;
        }
        else if(l == "<Jump>"){
            StatePointer first = currentLineBeginState;
            int destinationLineNumber = parseInteger(root->children[1]);
            postponedTransitionBuffer.emplace_back(first, destinationLineNumber);
            //next line prep
            StatePointer successor = getNextLineStartState();
        }
        else if(l == "<ReadCondition>"){
            StatePointer first = currentLineBeginState;
            StatePointer standardDestination = getNextLineStartState();
            string symbolName = parseSymbolLiteral(root->children[3]);
            postponedTransitionBuffer.emplace_back(first, standardDestination, set<string>{symbolName});
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            postponedTransitionBuffer.emplace_back(first, conditionalDestinationLineNumber, set<string>{symbolName}, true);
        }
        else if(l == "<ConditionalMove>"){
            StatePointer first = currentLineBeginState;
            TMTapeDirection direction = parseDirection(root->children.at(3));
            auto symbols = parseIdentifierList(root->children.at(1));
            postponedTransitionBuffer.emplace_back(first, first, symbols);
            postponedTransitionBuffer.back().directions[0] = direction;
            StatePointer conditionalDestination = getNextLineStartState();
            postponedTransitionBuffer.emplace_back(first, conditionalDestination, symbols, true);
        }
        else if(l == "<Accept>"){
            StatePointer first = currentLineBeginState;
            StatePointer destination = makeState(0, true);
            postponedTransitionBuffer.emplace_back(first, destination);
            StatePointer nextLineDummy = getNextLineStartState();
        }
        else if(l == "<ImmediateSymbolValueAssignment>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            StatePointer destination = getNextLineStartState();
            currentIntoVariable(variableName, first, destination, 0, variableContainingIndex);
        }
        else if(l == "<SymbolValueAssignment>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            string variableValue = parseSymbolLiteral(root->children[1]);
            StatePointer destination = getNextLineStartState();
            StatePointer writer = MoveToVariableValue(first, variableName, variableContainingIndex);
            // option 1: variable name found: overwrite current value, whatever it is
                postponedTransitionBuffer.emplace_back(writer, writer, set<string>{variableValue, VariableTapeEnd});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().toWrite = variableValue;
                postponedTransitionBuffer.emplace_back(writer, destination);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Right;
            // option 2: tape end found: overwrite it and put a new tape end to the right
                StatePointer writeName = makeState();
                //write the name
                postponedTransitionBuffer.emplace_back(writer, writeName, set<string>{VariableTapeEnd}, true);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().toWrite = variableName;
                postponedTransitionBuffer.back().directions[1] = Right;
                StatePointer writeValue = makeState();
                postponedTransitionBuffer.emplace_back(writeName, writeValue);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().toWrite = variableValue;
                postponedTransitionBuffer.back().directions[1] = Right;
                postponedTransitionBuffer.emplace_back(writeValue, writeValue, set<string>{VariableTapeEnd});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().toWrite = VariableTapeEnd;
                postponedTransitionBuffer.emplace_back(writeValue, destination, set<string>{VariableTapeEnd}, true);
                postponedTransitionBuffer.back().tape = 1;
        }
        else if(l == "<SymbolVariableCondition>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[5]);
            string variableValue = parseSymbolLiteral(root->children[3]);
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            //search for the tape begin marker
            StatePointer goLeft = makeState();
            postponedTransitionBuffer.emplace_back(first, goLeft);
            postponedTransitionBuffer.emplace_back(goLeft, goLeft, set<string>{VariableTapeStart});
            postponedTransitionBuffer.back().directions[1] = Left;
            postponedTransitionBuffer.back().tape = 1;
            //move right until variable name or tape end found
            StatePointer goRight = makeState();
            postponedTransitionBuffer.emplace_back(goLeft, goRight, set<string>{VariableTapeStart}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(goRight, goRight, set<string>{variableName, VariableTapeEnd});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;

            // move to look at the value
            StatePointer observe = makeState();
            postponedTransitionBuffer.emplace_back(goRight, observe);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            //regular condition logic
            StatePointer standardDestination = getNextLineStartState();
            postponedTransitionBuffer.emplace_back(observe, standardDestination, set<string>{variableValue});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.emplace_back(observe, conditionalDestinationLineNumber, set<string>{variableValue}, true);
            postponedTransitionBuffer.back().tape = 1;

        }else if(l == "<IntegerValueAssignment>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            int assignedValue = parseInteger(root->children[1]);
            std::string binaryAssignedValue = IntegerAsBitString(assignedValue);
            StatePointer destination = getNextLineStartState();
            integerAssignment(variableName, binaryAssignedValue, first, destination, variableContainingIndex);
        }
        else if(l == "<IntegerVariableCondition>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[5]);
            int comparedValue = parseInteger(root->children[3]);
            std::string binaryComparedValue = IntegerAsBitString(comparedValue);
            StatePointer standardDestination = getNextLineStartState();
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);
            IntegerCompare(variableName, binaryComparedValue, standardDestination,
                           conditionalDestinationLineNumber, first, nullptr, variableContainingIndex);
        }
        else if(l == "<ImmediateAddition>" || l == "<ImmediateSubtraction>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            int addedValue = parseInteger(root->children[1]);
            std::string binaryAddedValue = IntegerAsBitString(addedValue, l == "<ImmediateSubtraction>");
            StatePointer destination = getNextLineStartState();
            immediateAddition(variableName, binaryAddedValue, first, destination, variableContainingIndex);
        }
        else if(l == "<BinaryAddition>" || l == "<BinarySubtraction>"){
            StatePointer first = currentLineBeginState;
            //seek second variable and copy to third tape
            auto [assignedVariableName, assignedVariableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            auto [readVariableName, readVariableContainingIndex] = parseVariableLocationContainer(root->children[1]);
            StatePointer destination = getNextLineStartState();
            StatePointer goRight = MoveToVariableValue(first, readVariableName, readVariableContainingIndex);
            StatePointer firstWriterState;
            if(assignedVariableContainingIndex == ""){
                StatePointer doneCopying = copyIntegerToThirdTape(goRight, true);
                // seek first variable
                StatePointer doneMoving = MoveToVariableValue(doneCopying, assignedVariableName, assignedVariableContainingIndex);
                firstWriterState = doneMoving;
            } else{
                StatePointer doneCopying = copyIntegerToThirdTape(goRight, false);
                StatePointer thirdTapeSeparator = makeState();
                postponedTransitionBuffer.emplace_back(doneCopying, thirdTapeSeparator);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Right;
                // seek first variable
                StatePointer doneMoving = MoveToVariableValue(thirdTapeSeparator, assignedVariableName, assignedVariableContainingIndex);
                StatePointer returnToBackOfBackOfSecondTerm = makeState();
                postponedTransitionBuffer.emplace_back(doneMoving, returnToBackOfBackOfSecondTerm);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Left;
                StatePointer returnToBackOfSecondTerm = makeState();
                postponedTransitionBuffer.emplace_back(returnToBackOfBackOfSecondTerm, returnToBackOfSecondTerm);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Left;
                postponedTransitionBuffer.emplace_back(returnToBackOfSecondTerm, returnToBackOfSecondTerm, set<string>{"0", "1"}, true);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Left;
                StatePointer backAtStartOfSecondTerm = makeState();
                postponedTransitionBuffer.emplace_back(returnToBackOfSecondTerm, backAtStartOfSecondTerm, std::set<string>{"B"}, true);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Right;
                firstWriterState = backAtStartOfSecondTerm;
            }

            //move to look at the value
            // start adding
            vector<StatePointer> writeValueStates = {firstWriterState, makeState()}; // first carry state cannot be reached here but makes the loop below easier
            addThirdToSecond(writeValueStates, l == "<BinarySubtraction>");
            //move back to start of third tape and erase
            StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
            StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
            StatePointer eraser = makeState();

            postponedTransitionBuffer.emplace_back(oldNormalState, eraser);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(oldCarryState, eraser);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;

            postponedTransitionBuffer.emplace_back(eraser, eraser, set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.back().toWrite = "B";

            postponedTransitionBuffer.emplace_back(eraser, destination, set<string>{"B"}, true);
        }
        else if(l == "<BinaryVariableCondition>"){
            StatePointer first = currentLineBeginState;

            auto [leftVariableName, leftVariableContainingIndex] = parseVariableLocationContainer(root->children[5]);
            auto [rightVariableName, rightVariableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            StatePointer standardDestination = getNextLineStartState();
            int conditionalDestinationLineNumber = parseInteger(root->children[1]);

            // seek second variable and copy it to third tape
            StatePointer moveToValue = MoveToVariableValue(first, rightVariableName, rightVariableContainingIndex);

            // if integer
            StatePointer doneCopying = copyIntegerToThirdTape(moveToValue, true);
            StatePointer moveToValue2 = MoveToVariableValue(doneCopying, leftVariableName, leftVariableContainingIndex);
            vector<StatePointer> checkValueStates = {moveToValue2};

            StatePointer falseEraser = makeState();

            for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
                StatePointer oldState = *std::next(checkValueStates.end(), -1);
                StatePointer newState = makeState();
                checkValueStates.push_back(newState);
                transitions.insert({
                                           TransitionDomain(oldState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                           TransitionImage(newState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                   });
                transitions.insert({
                                           TransitionDomain(oldState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                           TransitionImage(newState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                   });
                transitions.insert({
                                           TransitionDomain(oldState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                           TransitionImage(falseEraser, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}, {Stationary, Stationary, Stationary, Stationary})
                                   });
                transitions.insert({
                                           TransitionDomain(oldState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                           TransitionImage(falseEraser, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}, {Stationary, Stationary, Stationary, Stationary})
                                   });
            }
            // erase if false
            postponedTransitionBuffer.emplace_back(falseEraser, falseEraser, set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Right;
            postponedTransitionBuffer.back().toWrite = "B";
            StatePointer falseEraser2 = makeState();
            postponedTransitionBuffer.emplace_back(falseEraser, falseEraser2, set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            StatePointer falseEraser3 = makeState();
            postponedTransitionBuffer.emplace_back(falseEraser2, falseEraser3, set<string>{"0", "1", "B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.emplace_back(falseEraser3, falseEraser3, set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.emplace_back(falseEraser3, standardDestination, set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Stationary;

            //erase if true
            StatePointer trueEraser = makeState();
            postponedTransitionBuffer.emplace_back(*std::next(checkValueStates.end(), -1), trueEraser);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(trueEraser, trueEraser, set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.emplace_back(trueEraser, conditionalDestinationLineNumber, set<string>{"B"}, true);

            //if symbol
            //copy to third tape
            StatePointer doneCopying2 = makeState();
            for(const string& copiedSymbol: tapeAlphabet){
                if(copiedSymbol == "0" || copiedSymbol == "1") continue;
                transitions.insert({
                                           TransitionDomain(moveToValue, {SYMBOL_ANY, copiedSymbol, "B", SYMBOL_ANY}),
                                           TransitionImage(doneCopying2, {SYMBOL_ANY, copiedSymbol, copiedSymbol, SYMBOL_ANY}, {Stationary, Right, Stationary, Stationary})
                                   });
            }
            //move to the value
            StatePointer checkSingleValue = MoveToVariableValue(doneCopying2, leftVariableName);

            StatePointer symbolTrueIntermediate = makeState();
            postponedTransitionBuffer.emplace_back(symbolTrueIntermediate, conditionalDestinationLineNumber);
            for(const string& comparedSymbol1: tapeAlphabet){
                if(comparedSymbol1 == "0" || comparedSymbol1 == "1") continue;
                transitions.insert({
                                           TransitionDomain(checkSingleValue, {SYMBOL_ANY, comparedSymbol1, comparedSymbol1, SYMBOL_ANY}),
                                           TransitionImage(symbolTrueIntermediate, {SYMBOL_ANY, comparedSymbol1, "B", SYMBOL_ANY}, {Stationary, Stationary, Stationary, Stationary})
                                   });
                for(const string& comparedSymbol2: tapeAlphabet){
                    if(comparedSymbol2 == "0" || comparedSymbol2 == "1") continue;
                    if(comparedSymbol1 != comparedSymbol2){
                        transitions.insert({
                                                   TransitionDomain(checkSingleValue, {SYMBOL_ANY, comparedSymbol1, comparedSymbol2, SYMBOL_ANY}),
                                                   TransitionImage(standardDestination, {SYMBOL_ANY, comparedSymbol1, "B", SYMBOL_ANY}, {Stationary, Stationary, Stationary, Stationary})
                                           });

                    }
                }
            }
        }
        else if(l == "<BinaryMultiplication>" || l == "<ImmediateMultiplication>"){
            StatePointer first = currentLineBeginState;
            auto [assignedVariableName, assignedVariableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            StatePointer destination = getNextLineStartState();
            StatePointer sysVarLoaded;
            if(l == "<ImmediateMultiplication>"){
                // write the multiplier to sysvar
                int multiplier = parseInteger(root->children[1]);
                std::string binaryMultiplier = IntegerAsBitString(multiplier);
                StatePointer moveToVTB = makeState();
                postponedTransitionBuffer.emplace_back(first, moveToVTB, std::set<string>{VariableTapeStart});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Left;
                postponedTransitionBuffer.emplace_back(moveToVTB, moveToVTB, std::set<string>{VariableTapeStart});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Left;

                StatePointer writer1 = makeState();
                postponedTransitionBuffer.emplace_back(moveToVTB, writer1, set<string>{VariableTapeStart}, true);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Right;
                vector<StatePointer> writeValueStates1 = { writer1};
                for (char c : binaryMultiplier) {
                    writeValueStates1.emplace_back(makeState());
                    auto last = std::next(writeValueStates1.end(), -1);
                    auto penultimate = std::next(last, -1);
                    postponedTransitionBuffer.emplace_back(*penultimate, *last);
                    postponedTransitionBuffer.back().tape = 1;
                    postponedTransitionBuffer.back().toWrite = c;
                    postponedTransitionBuffer.back().directions[1] = Right;
                }
                sysVarLoaded = *std::next(writeValueStates1.end(), -1);
            }
            else{
                auto [readVariableName, readVariableContainingIndex] = parseVariableLocationContainer(root->children[1]);
                // copy the multiplier to third tape
                StatePointer moveToValue = MoveToVariableValue(first, readVariableName, readVariableContainingIndex);
                StatePointer doneCopying = copyIntegerToThirdTape(moveToValue, true);
                // copy the multiplier to sysvar
                StatePointer moveToVTB = makeState();
                postponedTransitionBuffer.emplace_back(doneCopying, moveToVTB, std::set<string>{VariableTapeStart});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Left;
                postponedTransitionBuffer.emplace_back(moveToVTB, moveToVTB, std::set<string>{VariableTapeStart});
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Left;

                vector<StatePointer> writeValueStates = {makeState()};
                postponedTransitionBuffer.emplace_back(moveToVTB, writeValueStates[0], set<string>{VariableTapeStart}, true);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Right;

                for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
                    StatePointer oldState = *std::next(writeValueStates.end(), -1);
                    StatePointer newState = makeState();
                    writeValueStates.push_back(newState);
                    transitions.insert({
                                               TransitionDomain(oldState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                               TransitionImage(newState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                               TransitionImage(newState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                               TransitionImage(newState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                       });
                    transitions.insert({
                                               TransitionDomain(oldState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                               TransitionImage(newState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                                       });
                }

                StatePointer eraser = makeState();
                postponedTransitionBuffer.emplace_back(*std::next(writeValueStates.end(), -1), eraser);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Left;
                // erase multiplier from third tape
                postponedTransitionBuffer.emplace_back(eraser, eraser, set<string>{"0", "1"}, true);
                postponedTransitionBuffer.back().tape = 2;
                postponedTransitionBuffer.back().directions[2] = Left;
                postponedTransitionBuffer.back().toWrite = "B";
                sysVarLoaded = eraser;

            }

            // put multiplicand on third tape
            StatePointer moveToValue2 = MoveToVariableValue(sysVarLoaded, assignedVariableName, assignedVariableContainingIndex);
            StatePointer doneCopying2 = copyIntegerToThirdTape(moveToValue2, true);
            // erase multiplicand from second tape to start from 0 properly
            StatePointer eraseMultiplicand = makeState();
            postponedTransitionBuffer.emplace_back(doneCopying2, eraseMultiplicand, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.emplace_back(eraseMultiplicand, eraseMultiplicand, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "0";
            //check counter != 0
            StatePointer moveToVTB2 = makeState();
            postponedTransitionBuffer.emplace_back(eraseMultiplicand, moveToVTB2, std::set<string>{VariableTapeStart});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Left;
            postponedTransitionBuffer.emplace_back(moveToVTB2, moveToVTB2, std::set<string>{VariableTapeStart});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Left;
            StatePointer checkIsNotZero = makeState();
            StatePointer prepareCounterDecrement = makeState();
            StatePointer multiplicationDone = makeState();
            postponedTransitionBuffer.emplace_back(moveToVTB2, checkIsNotZero, std::set<string>{VariableTapeStart}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, checkIsNotZero, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, prepareCounterDecrement, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.emplace_back(checkIsNotZero, multiplicationDone, std::set<string>{VariableTapeStart, "0", "1"});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;

            // decrement counter
            // move to counter start
            postponedTransitionBuffer.emplace_back(prepareCounterDecrement, prepareCounterDecrement, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Left;
            StatePointer counterDecrement = makeState();
            postponedTransitionBuffer.emplace_back(prepareCounterDecrement, counterDecrement, std::set<string>{VariableTapeStart}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            StatePointer counterDecrementBorrowing = makeState();
            StatePointer doneDecrementing = makeState();
            postponedTransitionBuffer.emplace_back(counterDecrement, doneDecrementing, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.emplace_back(counterDecrement, counterDecrementBorrowing, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, counterDecrementBorrowing, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, doneDecrementing, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "0";

            StatePointer moveBackToMultiplicand = makeState();
            postponedTransitionBuffer.emplace_back(doneDecrementing, moveBackToMultiplicand, std::set<string>{assignedVariableName});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(moveBackToMultiplicand, moveBackToMultiplicand, std::set<string>{assignedVariableName});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            StatePointer moveBackToMultiplicandValue = makeState();
            postponedTransitionBuffer.emplace_back(moveBackToMultiplicand, moveBackToMultiplicandValue, std::set<string>{assignedVariableName}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;


            // add once
            vector<StatePointer> writeValueStates2 = {moveBackToMultiplicandValue, makeState()};
            addThirdToSecond(writeValueStates2, false);
            StatePointer oldNormalState = *std::next(writeValueStates2.end(), -2);
            StatePointer oldCarryState = *std::next(writeValueStates2.end(), -1);
            //Tape head on third tape back to start
            StatePointer ThirdTapeBackToStart1 = makeState();
            StatePointer ThirdTapeBackToStart2 = makeState();
            postponedTransitionBuffer.emplace_back(oldNormalState, ThirdTapeBackToStart1, std::set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(oldCarryState, ThirdTapeBackToStart1, std::set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart1, ThirdTapeBackToStart2, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart2, ThirdTapeBackToStart2, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;

            // go back to check counter
            postponedTransitionBuffer.emplace_back(ThirdTapeBackToStart2, moveToVTB2, std::set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Right;

            // if multiplication done, erase third tape
            postponedTransitionBuffer.emplace_back(multiplicationDone, multiplicationDone, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Right;
            StatePointer eraser2 = makeState();
            postponedTransitionBuffer.emplace_back(multiplicationDone, eraser2, std::set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(eraser2, eraser2, std::set<string>{"0", "1"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.emplace_back(eraser2, destination, std::set<string>{"B"}, true);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().directions[2] = Left;
        }
        else if(l == "<ImmediateAnd>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[3]);
            int addedValue = parseInteger(root->children[1]);
            std::string binaryAndedValue = IntegerAsBitString(addedValue);
            StatePointer destination = getNextLineStartState();
            bitwiseAnd(variableName, binaryAndedValue, first, destination, variableContainingIndex);
        }
        else if(l == "<CellularAutomatonDeclaration>"){
            auto symbols = parseIdentifierList(root->children.at(4));
            if (!CAstart) CAstart = makeState();
            if (!CAend) CAend = makeState();
            StatePointer getDirections = makeState();
            postponedTransitionBuffer.emplace_back(CAstart, getDirections, symbols, true);
            // put all the directions in variables
            StatePointer previous = getDirections;
            for (auto &direction: map<std::pair<TMTapeDirection, TMTapeDirection>, string>{{{TMTapeDirection::Left,  TMTapeDirection::Right}, "Left"},
                                                                                           {{TMTapeDirection::Right, TMTapeDirection::Left},  "Right"},
                                                                                           {{TMTapeDirection::Up,    TMTapeDirection::Down},  "Up"},
                                                                                           {{TMTapeDirection::Down,  TMTapeDirection::Up},    "Down"},
                                                                                           {{TMTapeDirection::Front, TMTapeDirection::Back},  "Front"},
                                                                                           {{TMTapeDirection::Back,  TMTapeDirection::Front}, "Back"}}) {
                StatePointer move = makeState();
                tapeMove(direction.first.first, previous, move, 3);
                StatePointer store = makeState();
                currentIntoVariable(direction.second, move, store, 3);
                StatePointer goBack = makeState();
                tapeMove(direction.first.second, store, goBack, 3);
                previous = goBack;
            }
            StatePointer temporarilyHiddenDestination = currentLineBeginState;
            currentLineBeginState = previous;
            currentLineNumber++;
            explorer(root->children[2]);
            postponedTransitionBuffer.emplace_back(currentLineBeginState, CAend);
            registerRegularNewline(temporarilyHiddenDestination);

        }
        else if(l == "<CellularAutomatonRun>"){ // NEVER PUT THIS IN A SCRIPT MULTIPLE TIMES
            StatePointer first = currentLineBeginState;
            int z = parseInteger(root->children[1]);
            int y = parseInteger(root->children[3]);
            int x = parseInteger(root->children[5]);
            StatePointer destination = getNextLineStartState();

            // update the history tape
            StatePointer historyUpdated = makeState();
            updateHistoryTape(x, y, z, first, historyUpdated);
            // execute the CA
            doThingForEveryVoxelInCube(x, y, z, historyUpdated, destination, CAstart, CAend, {0,3});
        }
        else if(l == "<ArrayDeclaration>"){
            StatePointer first = currentLineBeginState;
            string arrayName = root->children[8]->token->lexeme;
            int arraySize = parseInteger(root->children[4]);
            int defaultValue = parseInteger(root->children[2]);
            StatePointer destination = getNextLineStartState();

            // go to tape end
            postponedTransitionBuffer.emplace_back(first, first, set<string>{VariableTapeEnd});
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            // write array name
            StatePointer arrayNameWriter = makeState();
            postponedTransitionBuffer.emplace_back(first, arrayNameWriter, set<string>{VariableTapeEnd}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = arrayName;
            postponedTransitionBuffer.back().directions[1] = Right;
            // write default value
            std::vector<StatePointer> writeValueStates = {arrayNameWriter};
            std::string binaryDefaultValue = IntegerAsBitString(defaultValue);
            for (int i = 0; i < arraySize; ++i) {
                for (char c : binaryDefaultValue) {
                    writeValueStates.emplace_back(makeState());
                    auto last = std::next(writeValueStates.end(), -1);
                    auto penultimate = std::next(last, -1);
                    postponedTransitionBuffer.emplace_back(*penultimate, *last);
                    postponedTransitionBuffer.back().tape = 1;
                    postponedTransitionBuffer.back().toWrite = c;
                    postponedTransitionBuffer.back().directions[1] = Right;
                }
            }
            // write tape end
            StatePointer arrayEndWriter = makeState();
            postponedTransitionBuffer.emplace_back(writeValueStates.back(), destination);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = VariableTapeEnd;
        }
        else if(l == "<RandomInteger>"){
            StatePointer first = currentLineBeginState;
            auto [variableName, variableContainingIndex] = parseVariableLocationContainer(root->children[6]);
            int width = parseInteger(root->children[2]);
            if(width >= BINARY_VALUE_WIDTH)
                throw runtime_error("random integer demanded of size larger than what fits in a single variable");
            StatePointer destination = getNextLineStartState();

            StatePointer moveToValue = MoveToVariableValue(first, variableName, variableContainingIndex);
            StatePointer writeTemplate0 = makeState();
            StatePointer writeTemplate1 = makeState();
            postponedTransitionBuffer.emplace_back(moveToValue, writeTemplate0);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.back().directions[2] = Right;
            postponedTransitionBuffer.emplace_back(writeTemplate0, writeTemplate1);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.back().directions[2] = Left;


            std::vector<StatePointer> writeValueStates = {writeTemplate1};
            for (int i = 0; i < width; ++i) {
                auto previous = std::next(writeValueStates.end(), -1);

                StatePointer choose = makeState();
                TMTapeProbabilisticDirection stay{{Stationary}, {1}};
                transitions.insert({
                                           TransitionDomain(*previous, {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY}),
                                           TransitionImage(choose, {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY}, {stay, stay, {{Stationary, Right}, {0.5, 0.5}}, stay})
                                   });
                StatePointer write = makeState();
                for (const string& pick: {"0", "1"}) {
                    transitions.insert({
                                               TransitionDomain(choose, {SYMBOL_ANY, SYMBOL_ANY, pick, SYMBOL_ANY}),
                                               TransitionImage(write, {SYMBOL_ANY, pick, SYMBOL_ANY, SYMBOL_ANY}, {Stationary, Stationary, pick == "1" ? Left : Stationary, Stationary})
                                       });
                }
                StatePointer shift = makeState();
                postponedTransitionBuffer.emplace_back(write, shift);
                postponedTransitionBuffer.back().tape = 1;
                postponedTransitionBuffer.back().directions[1] = Right;
                writeValueStates.push_back(shift);
            }
            auto last = *std::next(writeValueStates.end(), -1);
            StatePointer removeTemplate0 = makeState();
            StatePointer removeTemplate1 = makeState();
            postponedTransitionBuffer.emplace_back(last, removeTemplate0);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.back().directions[2] = Right;
            postponedTransitionBuffer.emplace_back(removeTemplate0, removeTemplate1);
            postponedTransitionBuffer.back().tape = 2;
            postponedTransitionBuffer.back().toWrite = "B";
            postponedTransitionBuffer.back().directions[2] = Left;
            postponedTransitionBuffer.emplace_back(removeTemplate1, destination);
        }
        else{
            cerr << "Instruction " << l << " is currently not supported by the compiler" << endl;
        }
    }
}

void TMGenerator::updateHistoryTape(int x, int y, int z, StatePointer &beginState, StatePointer &endState) {
    StatePointer start = makeState();
    StatePointer end = makeState();
    for(const auto& writtenSymbol: tapeAlphabet){
        transitions.insert({
                   TransitionDomain(start, {writtenSymbol, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY}),
                   TransitionImage(end, {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, writtenSymbol}, {Stationary, Stationary, Stationary, Stationary})
           });
    }
    doThingForEveryVoxelInCube(x, y, z, beginState, endState, start, end, {0,3});
}

void TMGenerator::doThingForEveryVoxelInCube(int x, int y, int z, StatePointer &beginState, StatePointer &destination,
                                             StatePointer thingStart, StatePointer thingEnd,
                                             const vector<int> &tapesToMove) {
    std::string xString = IntegerAsBitString(x);
    std::string yString = IntegerAsBitString(y);
    std::string zString = IntegerAsBitString(z);

    std::string increment = IntegerAsBitString(1);
    std::string decrement = IntegerAsBitString(1, true);
    StatePointer initX = makeState();
    StatePointer initY = makeState();
    std::string zero = IntegerAsBitString(0);
    integerAssignment("Xcounter", zero, beginState, initX);
    integerAssignment("Ycounter", zero, initX, initY);
    integerAssignment("Zcounter", zero, initY, thingStart);

    // from thingStart to thingEnd, the actual thing gets executed


    // move forward
    StatePointer moveForward = makeState();
    moveMultipleTapes(Front, thingEnd, moveForward, tapesToMove);
    // increment X var
    StatePointer incrementX = makeState();
    immediateAddition("Xcounter", increment, moveForward, incrementX);


    StatePointer yCheck = makeState();
    StatePointer zCheck = makeState();

    // check if X == x
    StatePointer shiftY = makeState();
    IntegerCompare("Xcounter", xString, thingStart, 0, incrementX, shiftY);
    //  if so, move right(increment Y) and back while decrementing X until X is zero
    StatePointer yIncrement = makeState();
    StatePointer startXReset = makeState();
    moveMultipleTapes(Right, shiftY, yIncrement, tapesToMove);
    immediateAddition("Ycounter", increment, yIncrement, startXReset);
    StatePointer xDecrement = makeState();
    immediateAddition("Xcounter", decrement, startXReset, xDecrement);
    StatePointer xMove = makeState();
    moveMultipleTapes(Back, xDecrement, xMove, tapesToMove);
    IntegerCompare("Xcounter", zero, startXReset, 0, xMove, yCheck);

    // check if Y == y
    StatePointer shiftZ = makeState();
    IntegerCompare("Ycounter", yString, thingStart, 0, yCheck, shiftZ);
    //  if so, move up(increment Z) and left while decrementing Y until Y is zero
    StatePointer zIncrement = makeState();
    StatePointer startYReset = makeState();
    moveMultipleTapes(Up, shiftZ, zIncrement, tapesToMove);
    immediateAddition("Zcounter", increment, zIncrement, startYReset);
    StatePointer yDecrement = makeState();
    immediateAddition("Ycounter", decrement, startYReset, yDecrement);
    StatePointer yMove = makeState();
    moveMultipleTapes(Left, yDecrement, yMove, tapesToMove);
    IntegerCompare("Ycounter", zero, startYReset, 0, yMove, zCheck);
    // check if Z == z
    StatePointer backDown = makeState();
    IntegerCompare("Zcounter", zString, thingStart, 0, zCheck, backDown);
    //  if so, move down while decrementing Z until Z is zero AND THEN EXIT
    StatePointer startZReset = makeState();
    StatePointer zDecrement = makeState();
    moveMultipleTapes(Down, backDown, startZReset, tapesToMove);
    immediateAddition("Zcounter", decrement, startZReset, zDecrement);
    StatePointer zMove = makeState();
    IntegerCompare("Zcounter", zero, backDown, 0, zDecrement, destination);
}

void
TMGenerator::integerAssignment(const string &variableName, string &binaryAssignedValue, StatePointer &beginState,
                               StatePointer &destination, const string &variableContainingIndex) {
    StatePointer goRight = MoveToVariableValue(beginState, variableName, variableContainingIndex);

    // option 1: variable name found: overwrite current value, whatever it is
    std::vector<StatePointer> writeValueStates1 = {goRight};
    for (char c : binaryAssignedValue) {
        writeValueStates1.emplace_back(makeState());
        auto last = std::next(writeValueStates1.end(), -1);
        auto penultimate = std::next(last, -1);
        postponedTransitionBuffer.emplace_back(*penultimate, *last, std::set<string>{VariableTapeEnd});
        postponedTransitionBuffer.back().tape = 1;
        postponedTransitionBuffer.back().toWrite = c;
        postponedTransitionBuffer.back().directions[1] = Right;
    }
    postponedTransitionBuffer.emplace_back(*std::next(writeValueStates1.end(), -1), destination);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Stationary;
    // option 2: tape end found: overwrite it and put a new tape end to the right
    StatePointer writeName = makeState();
    //write the name
    postponedTransitionBuffer.emplace_back(goRight, writeName, std::set<string>{VariableTapeEnd}, true);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().toWrite = variableName;
    postponedTransitionBuffer.back().directions[1] = Right;

    std::vector<StatePointer> writeValueStates2 = {writeName};
    for (char c : binaryAssignedValue) {
        writeValueStates2.emplace_back(makeState());
        auto last = std::next(writeValueStates2.end(), -1);
        auto penultimate = std::next(last, -1);
        postponedTransitionBuffer.emplace_back(*penultimate, *last);
        postponedTransitionBuffer.back().tape = 1;
        postponedTransitionBuffer.back().toWrite = c;
        postponedTransitionBuffer.back().directions[1] = Right;
    }
    StatePointer writeTapeEnd = makeState();
    postponedTransitionBuffer.emplace_back(*std::next(writeValueStates2.end(), -1), writeTapeEnd, std::set<string>{VariableTapeEnd});
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().toWrite = VariableTapeEnd;
    postponedTransitionBuffer.emplace_back(writeTapeEnd, destination, std::set<string>{VariableTapeEnd}, true);
    postponedTransitionBuffer.back().tape = 1;
}

void
TMGenerator::IntegerCompare(const string &variableName, string &binaryComparedValue, StatePointer &standardDestination,
                            int conditionalDestinationLineNumber, StatePointer beginState,
                            StatePointer conditionalEndState, const string &indexContainingVariable) {
    StatePointer reader1 = MoveToVariableValue(beginState, variableName, indexContainingVariable);

    //bitwise comparison
    std::vector<StatePointer> readerStates = {reader1};
    for (char c : binaryComparedValue) {
        readerStates.emplace_back(makeState());
        auto last = std::next(readerStates.end(), -1);
        auto penultimate = std::next(last, -1);
        postponedTransitionBuffer.emplace_back(*penultimate, *last, std::set<string>{string(1, c)}, true);
        postponedTransitionBuffer.back().tape = 1;
        postponedTransitionBuffer.back().directions[1] = Right;
        postponedTransitionBuffer.emplace_back(*penultimate, standardDestination, std::set<string>{string(1, c)});
        postponedTransitionBuffer.back().tape = 1;
        postponedTransitionBuffer.back().directions[1] = Stationary;
    }
    if(conditionalEndState == nullptr){
        postponedTransitionBuffer.emplace_back(*std::next(readerStates.end(), -1), conditionalDestinationLineNumber);
    }else{
        postponedTransitionBuffer.emplace_back(*std::next(readerStates.end(), -1), conditionalEndState);
    }
    postponedTransitionBuffer.back().tape = 1;
}

void
TMGenerator::immediateAddition(const string &variableName, string &binaryAddedValue, StatePointer &startingState,
                               StatePointer &destination, const string &variableContainingIndex) {
    StatePointer writer1 = MoveToVariableValue(startingState, variableName, variableContainingIndex);

    //start adding
    std::vector<StatePointer> writeValueStates = {writer1, makeState()}; // first carry state cannot be reached here but makes the loop below easier
    for (char c : binaryAddedValue) {
        StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
        StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
        StatePointer newNormalState = makeState();
        StatePointer newCarryState = makeState();
        writeValueStates.push_back(newNormalState);
        writeValueStates.push_back(newCarryState);
        if(c == '0'){
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldCarryState, newNormalState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.back().directions[1] = Right;
        }else if(c == '1'){
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldNormalState, newCarryState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldCarryState, newCarryState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "1";
            postponedTransitionBuffer.back().directions[1] = Right;
        }
    }
    StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
    StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
    postponedTransitionBuffer.emplace_back(oldNormalState, destination);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Stationary;
    postponedTransitionBuffer.emplace_back(oldCarryState, destination);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Stationary;
}
void TMGenerator::bitwiseAnd(const string &variableName, string &binaryAddedValue, StatePointer &startingState,
           StatePointer &destination, const string &variableContainingIndex){
    StatePointer writer1 = MoveToVariableValue(startingState, variableName, variableContainingIndex);

    //start comparison
    std::vector<StatePointer> writeValueStates = {writer1};
    for (char c : binaryAddedValue) {
        StatePointer oldNormalState = *std::next(writeValueStates.end(), -1);
        StatePointer newNormalState = makeState();
        writeValueStates.push_back(newNormalState);
        if(c == '0'){
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.back().toWrite = "0";
        }else if(c == '1'){
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"0"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().toWrite = "0";
            postponedTransitionBuffer.back().directions[1] = Right;
            postponedTransitionBuffer.emplace_back(oldNormalState, newNormalState, std::set<string>{"1"}, true);
            postponedTransitionBuffer.back().tape = 1;
            postponedTransitionBuffer.back().directions[1] = Right;
        }
    }
    StatePointer oldNormalState = *std::next(writeValueStates.end(), -1);
    postponedTransitionBuffer.emplace_back(oldNormalState, destination);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Stationary;
}
void
TMGenerator::tapeMove(TMTapeDirection direction, StatePointer &beginState, StatePointer &destination, int tapeIndex) {
    postponedTransitionBuffer.emplace_back(beginState, destination);
    postponedTransitionBuffer.back().directions[tapeIndex] = direction;
}

void TMGenerator::currentIntoVariable(const string &variableName, const StatePointer &beginState,
                                      const StatePointer &destination, int tapeIndex,
                                      const string &variableContainingIndex) {
    assert(tapeIndex >= 0 && tapeIndex != 1);
    StatePointer goRight = MoveToVariableValue(beginState, variableName, variableContainingIndex);
    for (const string& symbolToWrite: tapeAlphabet) {
        if(symbolToWrite == VariableTapeEnd) continue;
        // option 1: variable name found: overwrite current value, whatever it is
        StatePointer writer = makeState();
        for(const string& ignoredSymbol: tapeAlphabet){
            if(ignoredSymbol == VariableTapeEnd) continue;
            vector<string> replaced = {SYMBOL_ANY, ignoredSymbol, SYMBOL_ANY, SYMBOL_ANY};
            vector<string> replacement = {SYMBOL_ANY, symbolToWrite, SYMBOL_ANY, SYMBOL_ANY};
            *std::next(replaced.begin(), tapeIndex) = symbolToWrite;
            *std::next(replacement.begin(), tapeIndex) = symbolToWrite;
            transitions.insert({
                                       TransitionDomain(goRight, replaced),
                                       TransitionImage(writer, replacement, {Stationary, Stationary, Stationary, Stationary})
                               });
        }

        for(const string& ignoredSymbol: tapeAlphabet){
            if(ignoredSymbol == symbolToWrite) continue;
            vector<string> replaced = {SYMBOL_ANY, ignoredSymbol, SYMBOL_ANY, SYMBOL_ANY};
            vector<string> replacement = {SYMBOL_ANY, symbolToWrite, SYMBOL_ANY, SYMBOL_ANY};
            *std::next(replaced.begin(), tapeIndex) = symbolToWrite;
            transitions.insert({
                                       TransitionDomain(writer, replaced),
                                       TransitionImage(writer, replacement, {Stationary, Stationary, Stationary, Stationary})
                               });
        }
        vector<string> replaced3 = {SYMBOL_ANY, symbolToWrite, SYMBOL_ANY, SYMBOL_ANY};
        *std::next(replaced3.begin(), tapeIndex) = symbolToWrite;
        transitions.insert({
                                   TransitionDomain(writer, replaced3),
                                   TransitionImage(destination, replaced3, {Stationary, Right, Stationary, Stationary})
                           });
        // option 2: tape end found: overwrite it and put a new tape end to the right
        StatePointer writeName = makeState();
        //write the name
        vector<string> replaced4 = {SYMBOL_ANY, VariableTapeEnd, SYMBOL_ANY, SYMBOL_ANY};
        vector<string> replaced5 = {SYMBOL_ANY, variableName, SYMBOL_ANY, SYMBOL_ANY};
        *std::next(replaced4.begin(), tapeIndex) = symbolToWrite;
        *std::next(replaced5.begin(), tapeIndex) = symbolToWrite;
        transitions.insert({
                                   TransitionDomain(goRight, replaced4),
                                   TransitionImage(writeName, replaced5, {Stationary, Right, Stationary, Stationary})
                           });
        StatePointer writeValue = makeState();
        //write the value
        vector<string> replaced6 = {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY};
        vector<string> replaced7 = {SYMBOL_ANY, symbolToWrite, SYMBOL_ANY, SYMBOL_ANY};
        *std::next(replaced6.begin(), tapeIndex) = symbolToWrite;
        *std::next(replaced7.begin(), tapeIndex) = symbolToWrite;
        transitions.insert({
                                   TransitionDomain(writeName, replaced6),
                                   TransitionImage(writeValue, replaced7, {Stationary, Right, Stationary, Stationary})
                           });
        for(const string& ignoredSymbol: tapeAlphabet){
            if(ignoredSymbol == VariableTapeEnd) continue;
            vector<string> replaced8 = {SYMBOL_ANY, ignoredSymbol, SYMBOL_ANY, SYMBOL_ANY};
            vector<string> replaced9 = {SYMBOL_ANY, VariableTapeEnd, SYMBOL_ANY, SYMBOL_ANY};
            *std::next(replaced8.begin(), tapeIndex) = symbolToWrite;
            *std::next(replaced9.begin(), tapeIndex) = symbolToWrite;
            transitions.insert({
                                       TransitionDomain(writeValue, replaced8),
                                       TransitionImage(writeValue, replaced9, {Stationary, Stationary, Stationary, Stationary})
                               });
        }
        vector<string> replaced10 = {SYMBOL_ANY, VariableTapeEnd, SYMBOL_ANY, SYMBOL_ANY};
        *std::next(replaced10.begin(), tapeIndex) = symbolToWrite;
        transitions.insert({
                                   TransitionDomain(writeValue, replaced10),
                                   TransitionImage(destination, replaced10, {Stationary, Stationary, Stationary, Stationary})
                           });
    }
}

void TMGenerator::addThirdToSecond(vector<StatePointer> &writeValueStates, bool subtract) {
    for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
        StatePointer oldNormalState = *std::next(writeValueStates.end(), -2);
        StatePointer oldCarryState = *std::next(writeValueStates.end(), -1);
        StatePointer newNormalState = makeState();
        StatePointer newCarryState = makeState();
        writeValueStates.push_back(newNormalState);
        writeValueStates.push_back(newCarryState);
        if(!subtract){
            //no carry
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            //yes carry
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
        }else{
            //no carry
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldNormalState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            //yes carry
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "1", "0", SYMBOL_ANY}),
                                       TransitionImage(newNormalState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "0", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
            transitions.insert({
                                       TransitionDomain(oldCarryState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                       TransitionImage(newCarryState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                               });
        }
    }
}

// Assumes the tape head is already moved to the variable value
StatePointer TMGenerator::copyIntegerToThirdTape(StatePointer startState, bool backToStart) {
    //copy to third tape and return to start
    std::vector<StatePointer> copyStates = {startState};

    StatePointer doneCopying = makeState();

    for (int i = 0; i < BINARY_VALUE_WIDTH - 1; ++i) {
        copyStates.push_back(makeState());
        transitions.insert({
                                   TransitionDomain(copyStates[i], {SYMBOL_ANY, "0", "B", SYMBOL_ANY}),
                                   TransitionImage(copyStates[i+1], {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                           });
        transitions.insert({
                                   TransitionDomain(copyStates[i], {SYMBOL_ANY, "1", "B", SYMBOL_ANY}),
                                   TransitionImage(copyStates[i+1], {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Right, Right, Stationary})
                           });
        //prepare return
        if(backToStart){
            if(i == 0){
                transitions.insert({
                                           TransitionDomain(copyStates[i+1], {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                           TransitionImage(doneCopying, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Left, Left, Stationary})
                                   });
                transitions.insert({
                                           TransitionDomain(copyStates[i+1], {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                           TransitionImage(doneCopying, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Left, Left, Stationary})
                                   });
            }else{
                transitions.insert({
                                           TransitionDomain(copyStates[i+1], {SYMBOL_ANY, "0", "0", SYMBOL_ANY}),
                                           TransitionImage(copyStates[i], {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Left, Left, Stationary})
                                   });
                transitions.insert({
                                           TransitionDomain(copyStates[i+1], {SYMBOL_ANY, "1", "1", SYMBOL_ANY}),
                                           TransitionImage(copyStates[i], {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Left, Left, Stationary})
                                   });
            }
        }
    }
    //the last one is special
    TMTapeDirection lastDirection = backToStart ? Stationary : Right;
    StatePointer lastState = backToStart ? copyStates[BINARY_VALUE_WIDTH-1] : makeState();
    transitions.insert({
                               TransitionDomain(copyStates[BINARY_VALUE_WIDTH - 1], {SYMBOL_ANY, "0", "B", SYMBOL_ANY}),
                               TransitionImage(lastState, {SYMBOL_ANY, "0", "0", SYMBOL_ANY}, {Stationary, Stationary, lastDirection, Stationary})
                       });
    transitions.insert({
                               TransitionDomain(copyStates[BINARY_VALUE_WIDTH - 1], {SYMBOL_ANY, "1", "B", SYMBOL_ANY}),
                               TransitionImage(lastState, {SYMBOL_ANY, "1", "1", SYMBOL_ANY}, {Stationary, Stationary, lastDirection, Stationary})
                       });
    if(!backToStart){
        return lastState;
    }
    return doneCopying;
}

StatePointer TMGenerator::MoveToVariableValue(StatePointer startState, const string &variableName,
                                              const string &variableContainingIndex) {
    StatePointer seekMainVariable;
    if(variableContainingIndex.empty()){
        seekMainVariable = startState;
    }else{ // for arrays
        StatePointer moved = MoveToVariableValue(startState, variableContainingIndex);
        StatePointer copied = copyIntegerToThirdTape(moved, true);
        seekMainVariable = copied;
    }
    //search for the tape begin marker
    StatePointer goLeft = makeState();
    postponedTransitionBuffer.emplace_back(seekMainVariable, goLeft);
    postponedTransitionBuffer.emplace_back(goLeft, goLeft, std::set<string>{VariableTapeStart, variableName});
    postponedTransitionBuffer.back().directions[1] = Left;
    postponedTransitionBuffer.back().tape = 1;
    // shortcut if the variable name is already found on the way
    StatePointer moveToValue = makeState();
    postponedTransitionBuffer.emplace_back(goLeft, moveToValue, std::set<string>{variableName}, true);
    postponedTransitionBuffer.back().directions[1] = Right;
    postponedTransitionBuffer.back().tape = 1;
    //move right until variable name or tape end found (stopping at tape end will halt unexpectedly so is better than going past the end)
    StatePointer goRight = makeState();
    postponedTransitionBuffer.emplace_back(goLeft, goRight, std::set<string>{VariableTapeStart}, true);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Right;
    postponedTransitionBuffer.emplace_back(goRight, goRight, std::set<string>{variableName, VariableTapeEnd});
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Right;
    postponedTransitionBuffer.emplace_back(goRight, moveToValue, set<string>{variableName}, true);
    postponedTransitionBuffer.back().tape = 1;
    postponedTransitionBuffer.back().directions[1] = Right;
    postponedTransitionBuffer.emplace_back(goRight, moveToValue, set<string>{VariableTapeEnd}, true);
    postponedTransitionBuffer.back().tape = 1;
    if(variableContainingIndex.empty()){
        return moveToValue;
    }
    // for arrays
    StatePointer returnToFront = makeState();
    StatePointer startErasing = makeState();
    StatePointer counterDecrement = makeState();
    StatePointer arrayIndexFound = makeState();
    //check if counter on third tape is zero and erase if so
    postponedTransitionBuffer.emplace_back(moveToValue, moveToValue, std::set<string>{"0"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.emplace_back(moveToValue, startErasing, std::set<string>{"B"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Left;
    postponedTransitionBuffer.emplace_back(startErasing, startErasing, std::set<string>{"0"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Left;
    postponedTransitionBuffer.back().toWrite = "B";
    postponedTransitionBuffer.emplace_back(startErasing, arrayIndexFound, std::set<string>{"B"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.emplace_back(moveToValue, returnToFront, std::set<string>{"1"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Left;
    postponedTransitionBuffer.emplace_back(returnToFront, returnToFront, std::set<string>{"0", "1"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Left;
    postponedTransitionBuffer.emplace_back(returnToFront, counterDecrement, std::set<string>{"B"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    //decrement counter
    StatePointer doneDecrementing = makeState();
    StatePointer counterDecrementBorrowing = makeState();
    postponedTransitionBuffer.emplace_back(counterDecrement, doneDecrementing, std::set<string>{"1"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.back().toWrite = "0";
    postponedTransitionBuffer.emplace_back(counterDecrement, counterDecrementBorrowing, std::set<string>{"0"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.back().toWrite = "1";
    postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, counterDecrementBorrowing, std::set<string>{"0"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.back().toWrite = "1";
    postponedTransitionBuffer.emplace_back(counterDecrementBorrowing, doneDecrementing, std::set<string>{"1"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    postponedTransitionBuffer.back().toWrite = "0";

    //and return to start of counter
    StatePointer startMoving = makeState();
    postponedTransitionBuffer.emplace_back(doneDecrementing, doneDecrementing, std::set<string>{"0", "1"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Left;
    postponedTransitionBuffer.emplace_back(doneDecrementing, startMoving, std::set<string>{"B"}, true);
    postponedTransitionBuffer.back().tape = 2;
    postponedTransitionBuffer.back().directions[2] = Right;
    //move one array element to the right
    std::vector<StatePointer> moveStates = {startMoving};
    for (int i = 0; i < BINARY_VALUE_WIDTH; ++i) {
        StatePointer previous = *std::next(moveStates.end(), -1);
        StatePointer next = makeState();
        moveStates.push_back(next);
        postponedTransitionBuffer.emplace_back(previous, next);
        postponedTransitionBuffer.back().tape = 1;
        postponedTransitionBuffer.back().directions[1] = Right;
    }
    //repeat
    StatePointer last = *std::next(moveStates.end(), -1);
    postponedTransitionBuffer.emplace_back(last, moveToValue);
    return arrayIndexFound;
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

void TMGenerator::moveMultipleTapes(TMTapeDirection direction, StatePointer &beginState, StatePointer &destination,
                                    const vector<int> tapeIndices) {
    vector<TMTapeDirection> directions(4, TMTapeDirection::Stationary);
    for (int i: tapeIndices) {
        directions[i] = direction;
    }
    transitions.insert({
                               TransitionDomain(beginState, {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY}),
                               TransitionImage(destination, {SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY, SYMBOL_ANY}, directions)
                       });
}

std::pair<string, string> TMGenerator::parseVariableLocationContainer(const shared_ptr<STNode> &root) {
    if(!root->children[0]->hasChildren()){ // just a variable name
        return {root->children[0]->token->lexeme, ""};
    }else{ // an array element
        shared_ptr<STNode> indirectPack = root->children[0];
        return {indirectPack->children[3]->token->lexeme, indirectPack->children[1]->token->lexeme};
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
