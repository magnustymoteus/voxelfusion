//
// Created by gruzi on 02/11/2023.
//

#include "TMTape.h"

#include <iostream>

template<class TMTapeElement>
TMTapeElement& getTapeElement(std::vector<TMTapeElement> &cells, const signed long &index) {
    const int size = static_cast<int> (cells.size());
    if(-size/2 > index || size/2 < index) {
        const unsigned int amountToInsert = std::abs(index) - (size / 2);
        cells.insert(cells.begin(), amountToInsert, TMTapeElement());
        cells.insert(cells.end(), amountToInsert, TMTapeElement());
        cells.resize(2*std::abs(index)+1);
    }
    return cells[cells.size()/2+index];
}

TMTapeCell & TMTape1D::operator[](const long &index) {
    return getTapeElement(cells, index);
}
TMTape1D & TMTape2D::operator[](const long &index) {
    return getTapeElement(cells, index);
}
TMTape2D &TMTape3D::operator[](const long &index) {
    return getTapeElement(cells, index);
}

void TMTape1D::replaceCurrentSymbol(const std::string &newSymbol) {
    cells[currentIndex].symbol = newSymbol;
}
void TMTape2D::replaceCurrentSymbol(const std::string &newSymbol) {
    cells[currentIndex].replaceCurrentSymbol(newSymbol);
}
void TMTape3D::replaceCurrentSymbol(const std::string &newSymbol) {
    cells[currentIndex].replaceCurrentSymbol(newSymbol);
}

std::string TMTape1D::getCurrentSymbol() const {
    return cells[currentIndex].symbol;
}
std::string TMTape2D::getCurrentSymbol() const {
    return cells[currentIndex].getCurrentSymbol();
}
std::string TMTape3D::getCurrentSymbol() const {
    return cells[currentIndex].getCurrentSymbol();
}

bool TMTape1D::moveTapeHead(const TMTapeDirection &direction) {
    const int add = (direction == Right) ? 1 : (direction == Left) ? -1 : 0;
    currentIndex += add;
    return add || direction == Stationary;
}
bool TMTape2D::moveTapeHead(const TMTapeDirection &direction) {
    if(!cells[currentIndex].moveTapeHead(direction)) {
        const int add = (direction == Up) ? 1 : (direction == Down) ? -1 : 0;
        currentIndex += add;
        return add;
    }
    return true;
}
bool TMTape3D::moveTapeHead(const TMTapeDirection &direction) {
    if(!cells[currentIndex].moveTapeHead(direction)) {
        const int add = (direction == Front) ? 1 : (direction == Back) ? -1 : 0;
        currentIndex += add;
        return add;
    }
    return true;
}

void TMTape1D::print() const {
    for(const TMTapeCell &currentCell : cells) {
        std::cout << currentCell.symbol << " ";
    }
    std::cout << std::endl;
}
void TMTape2D::print() const {
    for(const TMTape1D &currentCellRow : cells) {
        currentCellRow.print();
    }
    std::cout << std::endl;
}