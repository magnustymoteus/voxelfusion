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

    void insertIfNotASubset(std::set<std::string> &a, const std::set<std::string> &b, bool &hasChanged);

    void print(const std::vector<std::string> &body);

    void print(const ItemSet &itemSet);
}
#endif //CFG_UTILS_H
