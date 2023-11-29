//
#include "CFGUtils.h"

#include <iostream>
#include <algorithm>



void CFGUtils::print(const std::set<std::string> &set) {
    std::set<std::string> newSet = set;
    const bool containsEpsilon = newSet.find("") != newSet.end();
    newSet.erase("");
    std::string result = "{";
    const unsigned int size = newSet.size();
    unsigned int i=0;
    for(const std::string &currentSymbol : newSet) {
        if(!currentSymbol.empty()) result += currentSymbol;
        if(i!=size-1) result += ", ";
        i++;
    }
    if(containsEpsilon) result += ", ";
    std::cout << result+"}\n";
}
void CFGUtils::print(const std::map<std::string, std::set<std::string>> &sets) {
    std::cout << std::endl;
    for(const auto & currentVariable : sets) {
        std::cout << std::string(4, ' ') << currentVariable.first << ": ";
        print(currentVariable.second);
    }
}
void CFGUtils::print(const CFGProductionBodies &bodies) {
    for(const CFGProductionBody &currentBody : bodies) {
        print(currentBody);
    }
}
void CFGUtils::print(const std::vector<std::string> &body) {
    std::cout << "`";
    for(unsigned int i=0;i<body.size();i++) {
        std::cout << body[i];
        if(i < body.size()-1) std::cout << " ";
    }
    std::cout << "`\n";
}
void CFGUtils::print(const ItemSet &itemSet) {
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
            print(currentProductions.second.getLookaheads());
        }
    }
}
std::string CFGUtils::getString(const CFGProductionBody &body) {
    std::string str = "`";
    for(unsigned int i=0;i<body.size();i++) {
        str += body[i];
        if(i < body.size()-1) str+= " ";
    }
    return str+"`";
}
std::string CFGUtils::getString(const AugmentedProductionBody &augmentedBody) {
    std::string str = "`";
    for(unsigned int i=0;i<augmentedBody.getContent().size();i++) {
        if(augmentedBody.getReadingIndex() == i) str += '.';
        str += augmentedBody.getContent()[i];
        if(i < augmentedBody.getContent().size()-1) str+= " ";
    }
    return str+"`";
}
void CFGUtils::print(const AugmentedProductionBody &augmentedBody) {
    std::cout << getString(augmentedBody);
    std::cout << "`\n";
}
void CFGUtils::print(const AugmentedProductionBodies &augmentedBodies) {
    for(const AugmentedProductionBody &currentAugmentedBody : augmentedBodies) {
        print(currentAugmentedBody);
    }
}
void CFGUtils::print(const CFG &cfg) {
    std::cout << "V = ";
    CFGUtils::print(cfg.getVariables());

    std::cout << "T = ";
    CFGUtils::print(cfg.getTerminals());

    std::cout << "P = {\n";
    for (std::pair<std::string, CFGProductionBodies> currentRule: cfg.getProductionRules()) {
        std::sort(currentRule.second.begin(), currentRule.second.end());
        for (unsigned int i=0; i < currentRule.second.size(); i++) {
            std::cout << "\t" << currentRule.first << " -> `";
            for (unsigned int j=0; j < currentRule.second[i].size(); j++) {
                std::cout << currentRule.second[i][j];
                if(j+1 != currentRule.second[i].size()) std::cout << " ";
            }
            std::cout << "`\n";
        }
    }
    std::cout << "}\n";
    std::cout << "S = " << cfg.getStartingVariable();
    std::cout << std::endl;

}
std::string CFGUtils::getCurrentlyReadSymbol(const AugmentedProductionBody &body) {
    if(body.getReadingIndex() < body.getContent().size()) {
        return body.getContent()[body.getReadingIndex()];
    }
    return "";
}
// add rules to item set entry if they don't exist already
// returns true if the itemset entry has been changed at the end
bool CFGUtils::addToItemSet(ItemSet &itemSet, const std::pair<std::string, AugmentedProductions> &newProductions) {
    const std::string &head = newProductions.first;
    if(itemSet.find(head) == itemSet.end()) {
        itemSet.insert(newProductions);
        return true;
    }
    else {
        const AugmentedProductions &productionsRhs = newProductions.second;
        const std::set<std::string> &newLookaheads = newProductions.second.getLookaheads();
        AugmentedProductions &productions = itemSet.at(head);
        AugmentedProductionBodies newBodies;

        for(const AugmentedProductionBody &currentNewBody : productionsRhs.getBodies()) {
            bool insertCurrentNewBody = true;
            for(const AugmentedProductionBody &currentExistingBody : productions.getBodies()) {
                if(currentNewBody.getContent() == currentExistingBody.getContent() &&
                currentNewBody.getReadingIndex() == currentExistingBody.getReadingIndex()) {
                    insertCurrentNewBody = false;
                }
            }
            if(insertCurrentNewBody) newBodies.push_back(currentNewBody);
        }

        const unsigned int capturedBodiesSize = productions.getBodies().size();
        const unsigned int capturedLookaheadsSize = productions.getLookaheads().size();

        productions.bodies.insert(productions.bodies.begin(), newBodies.begin(), newBodies.end());
        productions.lookaheads.insert(newLookaheads.begin(), newLookaheads.end());


        return capturedBodiesSize != productions.getBodies().size() ||
            capturedLookaheadsSize != productions.getLookaheads().size();
    }
}

bool CFGUtils::equalCoreWise(const ItemSet &itemSet1, const ItemSet &itemSet2) {
    if(itemSet1.size() != itemSet2.size()) return false;
    // checks if all productions are equal core-wise (grammar body is equal but not necessarily the lookaheads)
    return std::all_of(itemSet1.begin(), itemSet1.end(), [&]( const std::pair<std::string, AugmentedProductions> &currentProductions) {
        const auto iter = itemSet2.find(currentProductions.first);
        if(iter == itemSet2.end()) return false;
        return currentProductions.second.isEqualCorewise(iter->second);
    });
}