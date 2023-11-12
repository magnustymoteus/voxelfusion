#include "CFG.h"
#include "AugmentedCFG.h"
#include "utils.h"
#include <iostream>

using namespace std;

int main() {
    CFG cfg("input/CFG.json");
    AugmentedCFG aCfg(cfg);
    return 0;
}