#include "CFG/AugmentedCFG.h"
#include "CFG/CFGUtils.h"
#include <iostream>

using namespace std;

int main() {
    AugmentedCFG aCfg("CFG/input/CFG.json");
    ItemSet nextItemSet;
    nextItemSet.insert({"T", aCfg.getItemSet().at("T")});
    CFGUtils::print(aCfg.getItemSet());
    std::cout << std::endl;
    CFGUtils::print(aCfg.computeClosure(nextItemSet));
    return 0;
}