#include "TMGenerator.h"

TMGenerator::TMGenerator(const set<string> &tapeAlphabet,
                         map<TransitionDomain, TransitionImage> &transitions,
                         set<StatePointer> &states) : tapeAlphabet(tapeAlphabet),
                                                                  transitions(transitions), states(states) {}

void TMGenerator::registerFollowingBeginState(StatePointer &state) {
    states.insert(state);
    currentState = state;
    lineStates[++currentStateNumber] = currentState;
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
    currentState = make_shared<const State>(to_string(currentStateNumber), true);
    states.insert(currentState);
    lineStates[0] = currentState;
    currentStateNumber++;

    explorer(root);
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
        cout << root->label << " " << root->token << endl;
        if(l == "<TapeMove>"){
            StatePointer destination = make_shared<const State>(to_string(currentStateNumber), false);
            TMTapeDirection direction = parseDirection(root->children[1]);
            for(auto& symbol: tapeAlphabet){
                transitions.insert({
                                            TransitionDomain(currentState, {symbol, "B"}),
                                            TransitionImage(destination, {symbol, "B"}, {direction, Stationary})
                                    });
            }
            registerFollowingBeginState(destination);
        }
        else if(l == "<TapeWrite>"){
            StatePointer destination = make_shared<const State>(to_string(currentStateNumber), false);
            string symbolName = root->children[1]->token->lexeme;
            for(auto& symbol: tapeAlphabet){
                transitions.insert({
                                            TransitionDomain(currentState, {symbol, "B"}),
                                            TransitionImage(destination, {symbolName, "B"}, {Stationary, Stationary})
                                    });
            }
            registerFollowingBeginState(destination);
        }
        else if(l == "<Jump>"){
            StatePointer destination = lineStates.at(parseInteger(root->children[1]));
            for(auto& symbol: tapeAlphabet){
                transitions.insert({
                                            TransitionDomain(currentState, {symbol, "B"}),
                                            TransitionImage(destination, {symbol, "B"}, {Stationary, Stationary})
                                    });
            }
        }
    }
}