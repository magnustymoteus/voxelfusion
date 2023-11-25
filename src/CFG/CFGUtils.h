//

#ifndef CFG_UTILS_H
#define CFG_UTILS_H

#include <set>
#include <string>
#include <map>
#include <vector>

#include "AugmentedCFG.h"

namespace CFGUtils {

    void print(const std::set<std::string> &set);

    void print(const std::map<std::string, std::set<std::string>> &sets);

    void print(const ItemSet &itemSet);

    void print(const CFGProductionBody &body);

    void print(const CFGProductionBodies &bodies);

    void print(const AugmentedProductionBody &augmentedBody);

    void print(const AugmentedProductionBodies &augmentedBodies);

    void print(const CFG &cfg);

    std::string getCurrentlyReadSymbol(const AugmentedProductionBody &body);

    bool addToItemSet(ItemSet &itemSet, const std::pair<std::string, AugmentedProductions> &newProductions);

    bool equalCoreWise(const ItemSet &itemSet1, const ItemSet &itemSet2);
}
#endif //CFG_UTILS_H
