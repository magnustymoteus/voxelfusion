//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_TMTAPE_H
#define MTMDTURINGMACHINE_TMTAPE_H

#include <memory>
#include <string>

struct TMTapeCell {
    std::string symbol;
    std::shared_ptr<TMTapeCell> left;
    std::shared_ptr<TMTapeCell> right;

    explicit TMTapeCell(const std::string &symbolArg) : symbol(symbolArg) {}
    explicit TMTapeCell(std::string &&symbolArg) : symbol(std::move(symbolArg)) {}
};

struct TMTapeCell2D : public TMTapeCell {
    std::shared_ptr<TMTapeCell2D> top;
    std::shared_ptr<TMTapeCell2D> bottom;

    explicit TMTapeCell2D(const std::string &symbolArg) : TMTapeCell(symbolArg) {}
    explicit TMTapeCell2D(std::string &&symbolArg) : TMTapeCell(std::move(symbolArg)) {}
};

class TMTapeCell3D : public TMTapeCell {
public:
    std::shared_ptr<TMTapeCell3D> front;
    std::shared_ptr<TMTapeCell3D> back;
    std::shared_ptr<TMTapeCell3D> top;
    std::shared_ptr<TMTapeCell3D> bottom;

    explicit TMTapeCell3D(const std::string &symbolArg) : TMTapeCell(symbolArg) {}
    explicit TMTapeCell3D(std::string &&symbolArg) : TMTapeCell(std::move(symbolArg)) {}
};

enum TMTapeDimension {Tape1D=1,Tape2D=2,Tape3D=3};
enum TMTapeDirection {Left,Right,Up,Down,Front,Back};

class TMTape {
public:
    std::shared_ptr<TMTapeCell> startCell;
    TMTapeDimension dimension;

    explicit TMTape(const TMTapeCell &startCell)
    : startCell(std::make_shared<TMTapeCell>(startCell)), dimension(Tape1D) {}
    explicit TMTape(const TMTapeCell2D &startCell)
    : startCell(std::make_shared<TMTapeCell2D>(startCell)), dimension(Tape2D) {}
    explicit TMTape(const TMTapeCell3D &startCell)
    : startCell(std::make_shared<TMTapeCell3D>(startCell)), dimension(Tape3D) {}

};

#endif //MTMDTURINGMACHINE_TMTAPE_H
