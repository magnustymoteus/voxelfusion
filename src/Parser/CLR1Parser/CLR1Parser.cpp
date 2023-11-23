//

#include "CLR1Parser.h"
#include <iostream>

CLR1Parser::CLR1Parser(const AugmentedCFG &augmentedCfg) {
    itemSets.push_back(augmentedCfg.getItemSet());
    bool changed = true;
    while(changed) {
        changed = false;
        unsigned int itemSetIndex = 0;
        for(const ItemSet &currentItemSet : itemSets) {
            const std::set<std::string> &grammarSymbols = augmentedCfg.getAllSymbols();
            for(const std::string &currentSymbol : grammarSymbols) {
                const ItemSet &newItemSet = augmentedCfg.computeGoto(currentItemSet, currentSymbol);
                bool foundInMap = itemSetTransitionMap.find(itemSetIndex) != itemSetTransitionMap.end();
                if(foundInMap)
                    foundInMap = itemSetTransitionMap.at(itemSetIndex).find(currentSymbol)
                            != itemSetTransitionMap.at(itemSetIndex).end();
                if(!newItemSet.empty() && !foundInMap) {
                    const auto found = std::find(itemSets.begin(), itemSets.end(), newItemSet);
                    if(found == itemSets.end()) {
                        itemSets.push_back(newItemSet);
                        itemSetTransitionMap[itemSetIndex].insert({currentSymbol, itemSets.size()-1});
                    }
                    else {
                        itemSetTransitionMap[itemSetIndex].insert({currentSymbol, found - itemSets.begin()});
                    }
                    changed = true;
                }
            }
            itemSetIndex++;
        }
    }
}