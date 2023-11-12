//
#include "utils.h"

#include <iostream>

void CFGUtils::printSet(const std::set<std::string> &set) {
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
void CFGUtils::printSets(const std::map<std::string, std::set<std::string>> &sets) {
    std::cout << std::endl;
    for(const auto & currentVariable : sets) {
        std::cout << std::string(4, ' ') << currentVariable.first << ": ";
        printSet(currentVariable.second);
    }
}
void CFGUtils::insertIfNotASubset(std::set<std::string> &a, const std::set<std::string> &b, bool &hasChanged) {
    const size_t sizeBefore = a.size();
    a.insert(b.begin(), b.end());
    hasChanged = hasChanged || sizeBefore != a.size();
}
void CFGUtils::printBody(const std::vector<std::string> &body) {
    std::cout << "`";
    for(unsigned int i=0;i<body.size();i++) {
        std::cout << body[i];
        if(i < body.size()-1) std::cout << " ";
    }
    std::cout << "`\n";
}
