//

#include "AugmentedCFG.h"
#include "CFG/CFGUtils.h"
#include <iostream>

bool AugmentedProductionBody::operator<(const AugmentedProductionBody &body) const {
    return getContent() < body.getContent();
}
bool AugmentedProductions::operator<(const AugmentedProductions &productions) const {
    return getBodies() < productions.getBodies();
}

AugmentedCFG::AugmentedCFG(const std::string &jsonPath) : CFG(jsonPath),
augmentedStartingVariable(getStartingVariable()+"'") {
    // add S' manually
    AugmentedProductionBody startingProductionBody({getStartingVariable()});
    AugmentedProductions startingProduction({startingProductionBody}, {EOS_MARKER});
    itemSet.insert({getStartingVariable(), startingProduction});
    const std::map<std::string, std::set<std::string>> &followSets = computeFollowSets();

    for(const auto &currentProductions : getProductionRules()) {
    std::vector<AugmentedProductionBody> bodies;
         for(const auto &currentBody : currentProductions.second) {
            AugmentedProductionBody body(currentBody);
            bodies.push_back(body);
        }
         AugmentedProductions productions(bodies, followSets.at(currentProductions.first));
         itemSet.insert({currentProductions.first, productions});
    }
}

ItemSet AugmentedCFG::computeClosure(const ItemSet &givenItemSet) const {
    ItemSet result = givenItemSet;
    bool changed = true;
    while(changed) {
        changed = false;
        for (const auto &currentProductions: result) {
            for (const auto &currentBody: currentProductions.second.getBodies()) {
                const std::string readSymbol = currentBody.getContent()[currentBody.getReadingIndex()];
                if (!isTerminal(readSymbol)) {
                    const AugmentedProductions productions(getProductionBodies(readSymbol));
                    changed = changed || CFGUtils::addToItemSet(result, {readSymbol, productions});
                }
            }
        }
    }
    return result;
}

std::string AugmentedCFG::getAugmentedStartingVariable() const {
    return augmentedStartingVariable;
}

ItemSet AugmentedCFG::getItemSet() const {
    return itemSet;
}

AugmentedProductions::AugmentedProductions(const CFGProductionBodies &givenBodies) : lookaheads({}) {
    std::vector<AugmentedProductionBody> newBodies;
    for(const CFGProductionBody &currentBody : givenBodies) {
        AugmentedProductionBody newBody(currentBody);
        newBodies.push_back(newBody);
    }
    bodies = newBodies;
}
