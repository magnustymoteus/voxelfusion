//

#include "AugmentedCFG.h"
#include "CFG/CFGUtils.h"
#include <iostream>


bool AugmentedProductionBody::operator<(const AugmentedProductionBody &body) const {
    return getContent() < body.getContent();
}
bool AugmentedProductionBody::operator==(const AugmentedProductionBody &other) const {
    return content == other.content && readingIndex == other.readingIndex;
}
bool AugmentedProductions::operator<(const AugmentedProductions &productions) const {
    return getBodies() < productions.getBodies();
}
bool AugmentedProductions::operator==(const AugmentedProductions &other) const {
    return isEqualCorewise(other) && (lookaheads == other.lookaheads);
}
bool AugmentedProductions::isEqualCorewise(const AugmentedProductions &other) const {
    return bodies == other.bodies;
}

AugmentedCFG::AugmentedCFG(const std::string &jsonPath) : CFG(jsonPath),
augmentedStartingVariable(getStartingVariable()+"'") {
    // add S' manually
    AugmentedProductionBody startingProductionBody({getStartingVariable()});
    AugmentedProductions startingProduction({startingProductionBody}, {EOS_MARKER});
    itemSet.insert({getAugmentedStartingVariable(), startingProduction});
    variables.insert(getAugmentedStartingVariable());

    for(const auto &currentProductions : getProductionRules()) {
    std::vector<AugmentedProductionBody> bodies;
         for(const auto &currentBody : currentProductions.second) {
            AugmentedProductionBody body(currentBody);
            bodies.push_back(body);
        }
         AugmentedProductions productions(bodies);
         itemSet.insert({currentProductions.first, productions});
    }
    itemSet = computeClosure(itemSet);
}

ItemSet AugmentedCFG::computeClosure(const ItemSet &givenItemSet) const {
    ItemSet result = givenItemSet;
    bool changed = true;
    while(changed) {
        changed = false;
        for (const auto &currentProductions: result) {
            for (const auto &currentBody: currentProductions.second.getBodies()) {
                const std::string currentVariable = CFGUtils::getCurrentlyReadSymbol(currentBody);
                    if (isVariable(currentVariable)) {
                        const CFGProductionBodies &currentVariableBodies = getProductionBodies(currentVariable);
                        for(const CFGProductionBody &currentVariableBody : currentVariableBodies) {
                            const std::vector<std::string> &afterCurrentVariable = [&]() {
                                const unsigned int &readingIndex = currentBody.getReadingIndex();
                                const std::vector<std::string>& bodyContent =  currentBody.getContent();
                                return std::vector<std::string>(bodyContent.begin()+readingIndex+1, bodyContent.end());
                            }();
                            std::set<std::vector<std::string>> concatenatedFirstElems;
                            for(const std::string &currentLookahead : currentProductions.second.lookaheads) {
                                std::vector<std::string> concatenatedFirstElem = afterCurrentVariable;
                                concatenatedFirstElem.push_back(currentLookahead);
                                concatenatedFirstElems.insert(concatenatedFirstElem);
                            }
                            for(const std::vector<std::string> &currentConcatElem : concatenatedFirstElems) {
                                    for (const std::string &currentTerminal: computeFirstSet(currentConcatElem[0])) {
                                        const AugmentedProductions &newProductions =
                                                AugmentedProductions({AugmentedProductionBody(currentVariableBody)},
                                                                     {currentTerminal});
                                        changed = changed ||
                                                  CFGUtils::addToItemSet(result, {currentVariable, newProductions});
                                    }
                            }
                        }
                    }
            }
        }
    }
    return result;
}
ItemSet AugmentedCFG::computeGoto(const ItemSet &givenItemSet, const std::string &givenSymbol) const {
    ItemSet result;
    for (const auto &currentProductions: givenItemSet) {
        for (auto currentBody: currentProductions.second.getBodies()) {
            if(CFGUtils::getCurrentlyReadSymbol(currentBody) == givenSymbol) {
                  currentBody.readingIndex++;
                  AugmentedProductions newElement({currentBody}, currentProductions.second.lookaheads);
                  CFGUtils::addToItemSet(result, {currentProductions.first, newElement});
            }
        }
    }
    return computeClosure(result);
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
