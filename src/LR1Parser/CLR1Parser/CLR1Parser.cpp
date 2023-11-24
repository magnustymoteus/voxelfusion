//

#include "CLR1Parser.h"
#include <iostream>
#include <exception>
#include "CFG/CFGUtils.h"


void CLR1Parser::createShiftActions() {
    for(unsigned int i=0;i<itemSets.size();i++) {
        if (itemSetTransitionMap.find(i) != itemSetTransitionMap.end()) {
            for (const auto &currentTransition: itemSetTransitionMap.at(i)) {
                if (augmentedCfg.isTerminal(currentTransition.first)) {
                    parseTable[i].actionMap[currentTransition.first] = std::make_unique<Shift>(
                            currentTransition.second);
                }
            }
        }
    }
}
void CLR1Parser::createGotos() {
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
void CLR1Parser::createReduceAndAcceptActions() {
    unsigned int i = 0;
    for(const ItemSet &currentItemSet : itemSets) {
        for (const auto &currentProductions: currentItemSet) {
            for (const auto &currentBody: currentProductions.second.getBodies()) {
                if (currentBody.readingIndex == currentBody.getContent().size()) {
                    for (const std::string &currentLookahead: currentProductions.second.getLookaheads()) {
                        bool found = parseTable.find(i) != parseTable.end();
                        if (found && parseTable.at(i).actionMap.find(currentLookahead)
                                     != parseTable.at(i).actionMap.end()) {
                            throw std::invalid_argument("Conflict in parsing table found!");
                        } else {
                            // Accept
                            if (currentProductions.first == augmentedCfg.getStartingVariable()
                                && currentLookahead == EOS_MARKER) {
                                std::cout << "Accept\n";
                                parseTable[i].actionMap[currentLookahead] = std::make_unique<Accept>();
                            }
                            // Reduce
                            else {
                                CFGUtils::print(currentBody);
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
void CLR1Parser::createParseTable() {
    createShiftActions();
    createGotos();
    createReduceAndAcceptActions();
}
CLR1Parser::CLR1Parser(const std::string &jsonPath) : augmentedCfg(AugmentedCFG(jsonPath)) {
    createItemSets();
    createParseTable();
}

void CLR1Parser::createItemSets() {
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