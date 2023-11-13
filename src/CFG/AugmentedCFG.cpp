//

#include "AugmentedCFG.h"


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


std::string AugmentedCFG::getAugmentedStartingVariable() const {
    return augmentedStartingVariable;
}

ItemSet AugmentedCFG::getItemSet() const {
    return itemSet;
}
