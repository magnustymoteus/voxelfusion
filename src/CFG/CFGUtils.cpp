//
#include "CFGUtils.h"

#include <iostream>

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
void CFGUtils::insertIfNotASubset(std::set<std::string> &a, const std::set<std::string> &b, bool &hasChanged) {
    const size_t sizeBefore = a.size();
    a.insert(b.begin(), b.end());
    hasChanged = hasChanged || sizeBefore != a.size();
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