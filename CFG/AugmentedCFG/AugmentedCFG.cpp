//

#include "AugmentedCFG.h"
#include "utils.h"
#include <iostream>

using namespace CFGUtils;


bool AugmentedProductionBody::operator<(const AugmentedProductionBody &body) const {
    return getContent() < body.getContent();
}
bool AugmentedProductions::operator<(const AugmentedProductions &productions) const {
    return getBodies() < productions.getBodies();
}

AugmentedCFG::AugmentedCFG(const CFG &cfg) : startingVariable(cfg.getStartingVariable()+"'"), grammar(cfg) {
    // add S' manually
    AugmentedProductionBody startingProductionBody({cfg.getStartingVariable()});
    AugmentedProductions startingProduction({startingProductionBody}, {EOS_MARKER});
    startingItemSet.insert({getStartingVariable(), startingProduction});
    const std::map<std::string, std::set<std::string>> &followSets = cfg.computeFollowSets();

    for(const auto &currentProductions : cfg.getProductionRules()) {
    std::vector<AugmentedProductionBody> bodies;
         for(const auto &currentBody : currentProductions.second) {
            AugmentedProductionBody body(currentBody);
            bodies.push_back(body);
        }
         AugmentedProductions productions(bodies, followSets.at(currentProductions.first));
         startingItemSet.insert({currentProductions.first, productions});
    }

    //compute item sets
    //startingItemSet = computeClosure({startingVariable, startingProductionBody}, startingItemSet);
    //computeLookaheads(startingItemSet);
    std::cout << "I_0:" << std::endl;
    AugmentedCFG::print(startingItemSet);
    std::cout << std::endl;
    //computeNextItemSets(startingItemSet);

}

void AugmentedCFG::print(const ItemSet &itemSet) {
    for(const auto& currentProductions : itemSet) {
        for(const auto& currentBody : currentProductions.second.getBodies()) {
            std::cout << currentProductions.first << " -> `";
            for(unsigned int i=0;i<currentBody.getContent().size();i++) {
                if(currentBody.getReadingIndex() == i) std::cout << ".";
                std::cout << currentBody.getContent()[i];
                if(i < currentBody.getContent().size()-1) std::cout << " ";
            }
            if(currentBody.getReadingIndex() == currentBody.getContent().size()) std::cout << ".";
            std::cout << "`, ";
            printSet(currentProductions.second.getLookaheads());
        }
    }
}

std::string AugmentedCFG::getStartingVariable() const {
    return startingVariable;
}

ItemSet AugmentedCFG::getStartingItemSet() const {
    return startingItemSet;
}
ItemSet AugmentedCFG::getItemSet(const ItemSet &itemSet, const std::string &symbol) const {
    return itemSetTransitionMap.at(itemSet).at(symbol);
}
