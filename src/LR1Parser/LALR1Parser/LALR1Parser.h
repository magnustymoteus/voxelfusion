

#ifndef VOXELFUSION_LALR1PARSER_H
#define VOXELFUSION_LALR1PARSER_H

#include "LR1Parser/LR1Parser.h"

class LALR1Parser : public LR1Parser {
protected:
    std::vector<std::set<unsigned int>> computeCoreWisePartitions() const;
    std::vector<ItemSet> computeMergedItemSets(const std::vector<std::set<unsigned int>> &partitions) const;
    std::map<unsigned int, std::map<std::string, unsigned int>> computeMinimizedTransitionMap(
            const std::vector<std::set<unsigned int>> &partitions) const;
    void minimizeItemSet();
public:
    explicit LALR1Parser(const std::string &jsonPath);
};


#endif //VOXELFUSION_LALR1PARSER_H
