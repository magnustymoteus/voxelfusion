//

#include "LR1Parser.h"
#include "LR1ParsingSpace.h"
#include <iostream>
#include <algorithm>
#include "CFG/CFGUtils.h"

std::shared_ptr<STNode> LR1Parser::parse(const std::vector<Token> &tokenizedInput) const {
    LR1ParsingSpace parsingSpace;
    unsigned int tokenIndex = 0;

    while (!parsingSpace.accepted) {
        const Token &currentToken = tokenizedInput[tokenIndex];
        unsigned currentState = parsingSpace.stateStack.top();

        const std::string currentTerminal = [&]() -> std::string {
            const auto& found = TokenMapping::terminals.find(currentToken.type);
            if(found != TokenMapping::terminals.end()) return found->second;
            return currentToken.lexeme;
        }();

       (*parseTable.at(currentState).actionMap.at(currentTerminal))(parsingSpace);

       /* if (action.find("shift") != std::string::npos) {
            stack.push(ASTNode(tokens[tokenIndex]));
            tokenIndex++;
        } else if (action.find("reduce") != std::string::npos) {
            ProductionRule production_rule = get_production_rule(action);
            int num_symbols_to_pop = production_rule.right.size();
            std::vector<ASTNode> popped_symbols;

            for (int i = 0; i < num_symbols_to_pop; ++i) {
                popped_symbols.push_back(stack.top());
                stack.pop();
            }

            ASTNode new_node(production_rule.left, popped_symbols);

            stack.push(new_node);
        }*/
    }

    return parsingSpace.nodeStack.top();
}

void LR1Parser::print() const {
    for(const auto &currentRow : parseTable) {
        for(const auto &currentActionEntry : currentRow.second.actionMap) {
            std::cout << "Action(" << currentRow.first << "," << currentActionEntry.first << ") = ";
            currentActionEntry.second->print();
        }
        for(const auto &currentGotoEntry : currentRow.second.gotoMap) {
            std::cout << "Goto(" << currentRow.first << "," << currentGotoEntry.first << ") = ";
            std::cout << currentGotoEntry.second << std::endl;
        }
    }
}

void LR1Parser::createShiftActions() {
    for(unsigned int i=0;i<itemSets.size();i++) {
        if (itemSetTransitionMap.find(i) != itemSetTransitionMap.end()) {
            for (const auto &currentTransition: itemSetTransitionMap.at(i)) {
                if (augmentedCfg.isTerminal(currentTransition.first)) {
                    parseTable[i].actionMap[currentTransition.first] = std::make_unique<Shift>(currentTransition.second);
                }
            }
        }
    }
}
void LR1Parser::createGotos() {
    for(unsigned int i=0;i<itemSets.size();i++) {
        if (itemSetTransitionMap.find(i) != itemSetTransitionMap.end()) {
            for (const auto &currentTransition: itemSetTransitionMap.at(i)) {
                if(augmentedCfg.isVariable(currentTransition.first)) {
                    parseTable[i].gotoMap[currentTransition.first] = currentTransition.second;
                }
            }
        }
    }
}
void LR1Parser::createReduceAndAcceptActions() {
    unsigned int i = 0;
    for(const ItemSet &currentItemSet : itemSets) {
        for (const auto &currentProductions: currentItemSet) {
            for (const auto &currentBody: currentProductions.second.getBodies()) {
                if (currentBody.readingIndex == currentBody.getContent().size()) {
                    for (const std::string &currentLookahead: currentProductions.second.getLookaheads()) {
                        bool found = parseTable.find(i) != parseTable.end();
                        if (found && parseTable.at(i).actionMap.find(currentLookahead)
                                     != parseTable.at(i).actionMap.end()) {
                            throw std::invalid_argument("Conflict in parsing table found: "+
                            parseTable.at(i).actionMap.at(currentLookahead)->getString());
                        } else {
                            // Accept
                            if (currentProductions.first == augmentedCfg.getAugmentedStartingVariable()
                                && currentLookahead == EOS_MARKER) {
                                parseTable[i].actionMap[currentLookahead] = std::make_unique<Accept>();
                            }
                                // Reduce
                            else {
                                parseTable[i].actionMap[currentLookahead] =
                                        std::make_unique<Reduce>(currentProductions.first, currentBody);
                            }
                        }
                    }
                }
            }
        }
        i++;
    }
}
void LR1Parser::createParseTable() {
    createShiftActions();
    createGotos();
    createReduceAndAcceptActions();
}
void LR1Parser::createItemSets() {
    itemSets.push_back(augmentedCfg.getItemSet());
    bool changed = true;
    while(changed) {
        changed = false;
        unsigned int i =0;
        const std::vector<ItemSet> itemSetsCopy = itemSets;
        for(const ItemSet &currentItemSet : itemSetsCopy) {
            const std::set<std::string> &grammarSymbols = augmentedCfg.getAllSymbols();
            for(const std::string &currentSymbol : grammarSymbols) {
                const ItemSet &newItemSet = augmentedCfg.computeGoto(currentItemSet, currentSymbol);
                bool foundInMap = itemSetTransitionMap.find(i) != itemSetTransitionMap.end();
                if(foundInMap)
                    foundInMap = itemSetTransitionMap.at(i).find(currentSymbol)
                                 != itemSetTransitionMap.at(i).end();
                if(!newItemSet.empty() && !foundInMap) {
                    const auto found = std::find(itemSets.begin(), itemSets.end(), newItemSet);
                    if(found == itemSets.end()) {
                        itemSets.push_back(newItemSet);
                        itemSetTransitionMap[i].insert({currentSymbol, itemSets.size()-1});
                    }
                    else {
                        itemSetTransitionMap[i].insert({currentSymbol, found - itemSets.begin()});
                    }
                    changed = true;
                }
            }
            i++;
        }
    }
}

LR1Parser::LR1Parser(const std::string &jsonPath) : augmentedCfg(AugmentedCFG(jsonPath)) {}