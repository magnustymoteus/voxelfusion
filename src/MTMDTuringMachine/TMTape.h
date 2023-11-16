//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_TMTAPE_H
#define MTMDTURINGMACHINE_TMTAPE_H

#include <vector>
#include "TMTapeCell.h"

enum TMTapeDimension {Tape1D=1,Tape2D=2,Tape3D=3};
enum TMTapeDirection {Left='L',Right='R',Up='U',Down='D',Front='F',Back='B', Stationary='S'};

class TMTape {
public:
    std::shared_ptr<TMTapeCell> tapeHead;
    TMTapeDimension dimension;

    explicit TMTape(const TMTapeCell &startCell)
    : tapeHead(std::make_shared<TMTapeCell>(startCell)), dimension(Tape1D) {}
    explicit TMTape(const TMTapeCell2D &startCell)
    : tapeHead(std::make_shared<TMTapeCell2D>(startCell)), dimension(Tape2D) {}
    explicit TMTape(const TMTapeCell3D &startCell)
    : tapeHead(std::make_shared<TMTapeCell3D>(startCell)), dimension(Tape3D) {}

    void replaceCurrentSymbol(const std::string &newSymbol) {tapeHead->symbol = newSymbol;}
    [[nodiscard]] TMTapeDimension getDimension() const {return dimension;}

};

typedef std::vector<TMTape> TMTapes;

#endif //MTMDTURINGMACHINE_TMTAPE_H
