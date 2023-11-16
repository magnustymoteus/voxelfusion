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
typedef std::vector<AugmentedProductionBody> AugmentedProductionBodies;

struct AugmentedProductions {
    AugmentedProductionBodies bodies;
    std::set<std::string> lookaheads;

    explicit AugmentedProductions(const std::vector<AugmentedProductionBody> &bodies, const std::set<std::string> &lookaheads={}) :
    bodies(bodies), lookaheads(lookaheads) {}
    explicit AugmentedProductions(const CFGProductionBodies &bodies);

    [[nodiscard]] std::vector<AugmentedProductionBody> getBodies() const {return bodies;}
    [[nodiscard]] std::set<std::string> getLookaheads() const {return lookaheads;}
    bool operator<(const AugmentedProductions &productions) const;
};

typedef std::map<std::string, AugmentedProductions> ItemSet;

// TODO: finish computeClosure
class AugmentedCFG : public CFG {
private:
    ItemSet itemSet;
    std::string augmentedStartingVariable;

public:
    explicit AugmentedCFG(const std::string &jsonPath);

    [[nodiscard]] std::string getAugmentedStartingVariable() const;
    [[nodiscard]] ItemSet getItemSet() const;
    [[nodiscard]] ItemSet computeClosure(const ItemSet &itemSet) const;
};


#endif //CFG_AUGMENTEDCFG_H
