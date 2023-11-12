//

#ifndef CFG_AUGMENTEDCFG_H
#define CFG_AUGMENTEDCFG_H

#include "CFG.h"

struct AugmentedProductionBody {
    CFGProductionBody content;
    unsigned int readingIndex;

    AugmentedProductionBody(const CFGProductionBody &content, const unsigned int &readingIndex) :
    content(content), readingIndex(readingIndex) {}
    explicit AugmentedProductionBody(const CFGProductionBody &content) : content(content), readingIndex(0) {}

    [[nodiscard]] CFGProductionBody getContent() const {return content;}
    [[nodiscard]] unsigned int getReadingIndex() const {return readingIndex;}

    bool operator<(const AugmentedProductionBody &body) const;
};
struct AugmentedProductions {
    std::vector<AugmentedProductionBody> bodies;
    std::set<std::string> lookaheads;

    AugmentedProductions(const std::vector<AugmentedProductionBody> &bodies, const std::set<std::string> &lookaheads) :
    bodies(bodies), lookaheads(lookaheads) {}

    [[nodiscard]] std::vector<AugmentedProductionBody> getBodies() const {return bodies;}
    [[nodiscard]] std::set<std::string> getLookaheads() const {return lookaheads;}
    bool operator<(const AugmentedProductions &productions) const;
};

typedef std::map<std::string, AugmentedProductions> ItemSet;

class AugmentedCFG {
private:
    ItemSet startingItemSet;
    std::map<ItemSet, std::map<std::string, ItemSet>> itemSetTransitionMap;
    std::string startingVariable;
    const CFG &grammar;


public:
    explicit AugmentedCFG(const CFG &cfg);

    [[nodiscard]] ItemSet getStartingItemSet() const;
    [[nodiscard]] std::string getStartingVariable() const;
    [[nodiscard]] ItemSet getItemSet(const ItemSet &itemSet, const std::string &symbol) const;

    static void print(const ItemSet &itemSet);
    static ItemSet computeClosure(const ItemSet &itemSet);
};


#endif //CFG_AUGMENTEDCFG_H
