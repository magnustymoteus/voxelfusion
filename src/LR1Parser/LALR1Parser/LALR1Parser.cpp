//

#include "LALR1Parser.h"
#include "CFG/CFGUtils.h"
#include <algorithm>
#include <iostream>

unsigned int getCorrespondingPartitionIndex(
        const std::vector<std::set<unsigned int>> &partitions, const unsigned int &itemSetIndex) {
    for(unsigned int i=0;i<partitions.size();i++) {
        if(partitions[i].find(itemSetIndex) != partitions[i].end()) return i;
    }
    throw std::invalid_argument("Cannot find corresponding partition index");
}
bool hasCorrespondingPartition(const std::vector<std::set<unsigned int>> &partitions, const unsigned int &itemSetIndex)
{
    return std::any_of(partitions.begin(), partitions.end(), [&](const std::set<unsigned int> &currentPartition) {
        return currentPartition.find(itemSetIndex) != currentPartition.end();
    });
}
std::vector<std::set<unsigned int>> LALR1Parser::computeCoreWisePartitions() const {
    std::vector<std::set<unsigned int>> itemSetPartitions;
    for(unsigned int i=0;i<itemSets.size();i++) {
        bool isSingleton = true;
        for(unsigned int j=0;j<itemSets.size();j++) {
            if(i != j && CFGUtils::equalCoreWise(itemSets[i], itemSets[j])) {
                bool found = false;
                isSingleton = false;
                for(std::set<unsigned int> &currentSet : itemSetPartitions) {
                    const auto &setIter = currentSet.find(i);
                    if(setIter != currentSet.end()) {
                        currentSet.insert(j);
                        found = true;
                        break;
                    }
                }
                if(!found) itemSetPartitions.push_back({i,j});
            }
        }
        if(isSingleton) itemSetPartitions.push_back({i});
    }
    return itemSetPartitions;
}
std::vector<ItemSet> LALR1Parser::computeMergedItemSets(const std::vector<std::set<unsigned int>> &partitions) const {
    std::vector<ItemSet> newItemSets;
    // merge item sets according to partitions
    for(const std::set<unsigned int> &currentSet : partitions) {
        ItemSet currentItemSet;
        for(const unsigned int &currentIndex : currentSet) {
            for(const auto& productions : itemSets[currentIndex]) {
                CFGUtils::addToItemSet(currentItemSet, productions);
            }
        }
        newItemSets.push_back(currentItemSet);
    }
    return newItemSets;
}

std::map<unsigned int, std::map<std::string, unsigned int>>
LALR1Parser::computeMinimizedTransitionMap(const std::vector<std::set<unsigned int>> &partitions) const {
    std::map<unsigned int, std::map<std::string, unsigned int>> newItemSetTransitionMap;
    for(const auto& currentTransitions : itemSetTransitionMap) {
        const unsigned int &domainPartitionIndex =
                getCorrespondingPartitionIndex(partitions, currentTransitions.first);
        for(const auto& currentTransition : currentTransitions.second) {
            const unsigned int &imagePartitionIndex =
                    getCorrespondingPartitionIndex(partitions, currentTransition.second);
            newItemSetTransitionMap[domainPartitionIndex][currentTransition.first] = imagePartitionIndex;
        }
    }
    return newItemSetTransitionMap;
}
void LALR1Parser::minimizeItemSet() {
    // item sets partitioned by core-wise equality
    std::vector<std::set<unsigned int>> itemSetsPartitions = computeCoreWisePartitions();
    itemSets = computeMergedItemSets(itemSetsPartitions);
    // change the transition map so it corresponds to merged item sets
    itemSetTransitionMap = computeMinimizedTransitionMap(itemSetsPartitions);
}

LALR1Parser::LALR1Parser(const std::string &jsonPath) : LR1Parser(jsonPath) {
    createItemSets();
    minimizeItemSet();
    createParseTable();
}