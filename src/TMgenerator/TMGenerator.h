#ifndef VOXELFUSION_TMGENERATOR_H
#define VOXELFUSION_TMGENERATOR_H


#include "../MTMDTuringMachine/MTMDTuringMachine.h"
#include "LR1Parser/SyntaxTree/SyntaxTree.h"

#include <memory>

using std::shared_ptr, std::make_shared, std::set, std::tuple, std::string, std::map, std::to_string,
        std::cout, std::endl, std::runtime_error;
#include <iostream>

template<class ...TMTapeType>
class TMGenerator {
    shared_ptr<MTMDTuringMachine<TMTapeType...>>& output;
    const set<string> &tapeAlphabet;
    const set<string> &inputAlphabet;
    const tuple<TMTapeType*...> &tapes;
    shared_ptr<FiniteControl> control;
    shared_ptr<map<TransitionDomain, TransitionImage>> transitions;
    void (*updateCallback) (const std::tuple<TMTapeType*...> &tapes,
                            const TransitionDomain &domain, const TransitionImage &image);
    shared_ptr<set<StatePointer>> states;
    map<int, StatePointer> lineStates;
    StatePointer currentState;
    int currentStateNumber = 0;

    void explorer(const shared_ptr<STNode>& root);

    void registerFollowingBeginState(StatePointer& state);

    TMTapeDirection parseDirection(const shared_ptr<STNode>& root);
    int parseInteger(const shared_ptr<STNode>& root);
public:
    TMGenerator(shared_ptr<MTMDTuringMachine<TMTapeType...>> &output, const set<string> &tapeAlphabet,
                const set<string> &inputAlphabet, const tuple<TMTapeType *...> &tapes,
                void (*updateCallback)(const tuple<TMTapeType *...> &, const TransitionDomain &,
                                       const TransitionImage &)) : output(output), tapeAlphabet(tapeAlphabet),
                                                                   inputAlphabet(inputAlphabet), tapes(tapes),
                                                                   updateCallback(updateCallback) {}
    void assembleTasm(const shared_ptr<STNode> root);
};


template<class... TMTapeType>
void TMGenerator<TMTapeType...>::registerFollowingBeginState(StatePointer &state) {
    states->insert(state);
    currentState = state;
    lineStates[++currentStateNumber] = currentState;
}

template<class... TMTapeType>
TMTapeDirection TMGenerator<TMTapeType...>::parseDirection(const shared_ptr<STNode> &root) {
    //<Direction> -> e.g. right
    if(root->label != "<Direction>"){
        throw runtime_error("This AST node does not contain a Direction");
    }
    char directionAlias = root->children[0]->token->lexeme[0] - 32; //lowercase to uppercase, please forgive me
    return (TMTapeDirection)directionAlias;
}

template<class... TMTapeType>
int TMGenerator<TMTapeType...>::parseInteger(const shared_ptr<STNode> &root) {
    if(root->token->type != TokenType::Token_Integer){
        throw runtime_error("This AST node does not contain an Integer");
    }
    return std::stoi(root->token->lexeme);
}


template<class... TMTapeType>
void TMGenerator<TMTapeType...>::assembleTasm(const shared_ptr<STNode> root) {
    states = make_shared<set<StatePointer>>();
    transitions = make_shared<map<TransitionDomain, TransitionImage>>();
    currentState = make_shared<const State>(to_string(currentStateNumber), true);
    states->insert(currentState);
    lineStates[0] = currentState;
    currentStateNumber++;

    explorer(root);
    control = make_shared<FiniteControl>(*states, *transitions);
    output = make_shared<MTMDTuringMachine<TMTapeType...>>(tapeAlphabet, inputAlphabet, tapes, *control, updateCallback);
}
template<class... TMTapeType>
void TMGenerator<TMTapeType...>::explorer(const shared_ptr<STNode> &root) {
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
                transitions->insert({
                        TransitionDomain(currentState, {symbol}),
                        TransitionImage(destination, {symbol}, {direction})
                });
            }
            registerFollowingBeginState(destination);
        }
        else if(l == "<TapeWrite>"){
            StatePointer destination = make_shared<const State>(to_string(currentStateNumber), false);
            string symbolName = root->children[1]->token->lexeme;
            for(auto& symbol: tapeAlphabet){
                transitions->insert({
                        TransitionDomain(currentState, {symbol}),
                        TransitionImage(destination, {symbolName}, {Stationary})
                });
            }
            registerFollowingBeginState(destination);
        }
        else if(l == "<Jump>"){
            StatePointer destination = lineStates.at(parseInteger(root->children[1]));
            for(auto& symbol: tapeAlphabet){
                transitions->insert({
                        TransitionDomain(currentState, {symbol}),
                        TransitionImage(destination, {symbol}, {Stationary})
                });
            }
        }
    }
}
#endif //VOXELFUSION_TMGENERATOR_H
