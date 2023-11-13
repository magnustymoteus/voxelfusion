#include "CFG/AugmentedCFG.h"
#include "CFG/CFGUtils.h"

using namespace std;

int main() {
    AugmentedCFG aCfg("CFG/input/CFG.json");
    CFGUtils::print(aCfg.computeClosure(aCfg.getItemSet()));
    return 0;
}