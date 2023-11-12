#include <iostream>

#include "MTMDTuringMachine.h"

int main() {
    TMTapeCell3D cell("S");
    TMTape tape(cell);
    MTMDTuringMachine tm(tape);

    std::cout << tape.dimension;

    return 0;
}
