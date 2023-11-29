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

    CFGProductionBody getContent() const {return content;}
    unsigned int getReadingIndex() const {return readingIndex;}

    bool operator<(const AugmentedProductionBody &other) const;
    bool operator==(const AugmentedProductionBody &other) const;
};
typedef std::vector<AugmentedProductionBody> AugmentedProductionBodies;

struct AugmentedProductions {
    AugmentedProductionBodies bodies;
    std::set<std::string> lookaheads;

    explicit AugmentedProductions(const std::vector<AugmentedProductionBody> &bodies, const std::set<std::string> &lookaheads={}) :
    bodies(bodies), lookaheads(lookaheads) {}
    explicit AugmentedProductions(const CFGProductionBodies &bodies);

    std::vector<AugmentedProductionBody> getBodies() const {return bodies;}
    std::set<std::string> getLookaheads() const {return lookaheads;}

    bool isEqualCorewise(const AugmentedProductions &other) const;

    bool operator<(const AugmentedProductions &other) const;
    bool operator==(const AugmentedProductions &other) const;
};


typedef std::map<std::string, AugmentedProductions> ItemSet;

class AugmentedCFG : public CFG {
private:
    ItemSet itemSet;
    std::string augmentedStartingVariable;

public:
    explicit AugmentedCFG(const std::string &jsonPath);
    AugmentedCFG() = default;

    std::string getAugmentedStartingVariable() const;
    ItemSet getItemSet() const;
    ItemSet computeClosure(const ItemSet &itemSet) const;
    ItemSet computeGoto(const ItemSet &itemSet, const std::string &symbol) const;
};


#endif //CFG_AUGMENTEDCFG_H
