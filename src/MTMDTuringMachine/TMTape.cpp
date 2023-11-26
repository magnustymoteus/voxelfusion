//
// Created by gruzi on 02/11/2023.
//

#include "TMTape.h"
#include "TMTapeUtils.h"

#include <iostream>

TMTapeCell & TMTape1D::operator[](const int &index) {return TMTapeUtils::getTapeElement(cells, index);}
TMTape1D & TMTape2D::operator[](const int &index) {return TMTapeUtils::getTapeElement(cells, index);}
TMTape2D &TMTape3D::operator[](const int &index) {return TMTapeUtils::getTapeElement(cells, index);}

TMTapeCell TMTape1D::at(const int &index) const {return TMTapeUtils::getTapeElementNoExpand(cells, index);}
TMTape1D TMTape2D::at(const int &index) const {return TMTapeUtils::getTapeElementNoExpand(cells, index);}
TMTape2D TMTape3D::at(const int &index) const {return TMTapeUtils::getTapeElementNoExpand(cells, index);}

unsigned int TMTape1D::getElementSize() const {return cells.size();}
unsigned int TMTape2D::getElementSize() const {return TMTapeUtils::getGreatestSize(cells);}
unsigned int TMTape3D::getElementSize() const {return TMTapeUtils::getGreatestSize(cells);}

void TMTape1D::replaceCurrentSymbol(const std::string &newSymbol) {
    (*this)[currentIndex].symbol = newSymbol;
}
void TMTape2D::replaceCurrentSymbol(const std::string &newSymbol) {
    (*this)[currentIndex].replaceCurrentSymbol(newSymbol);
}
void TMTape3D::replaceCurrentSymbol(const std::string &newSymbol) {
    (*this)[currentIndex].replaceCurrentSymbol(newSymbol);
}

std::string TMTape1D::getCurrentSymbol() const {
    return at(currentIndex).symbol;
}
std::string TMTape2D::getCurrentSymbol() const {
    return at(currentIndex).getCurrentSymbol();
}
std::string TMTape3D::getCurrentSymbol() const {
    return at(currentIndex).getCurrentSymbol();
}

bool TMTape1D::moveTapeHead(const TMTapeDirection &direction) {
    const int add = (direction == Right) ? 1 : (direction == Left) ? -1 : 0;
    currentIndex += add;
    (*this)[currentIndex];
    return add || direction == Stationary;
}
bool TMTape2D::moveTapeHead(const TMTapeDirection &direction) {
    TMTape1D &tape = (*this)[currentIndex];
    int add = 0;
    const bool &moved = tape.moveTapeHead(direction);
    if(!moved) {
        add = (direction == Down) ? 1 : (direction == Up) ? -1 : 0;
        currentIndex += add;
        (*this)[currentIndex];
    }
    for(const auto &currentTape : cells) {
        currentTape->currentIndex = tape.currentIndex;
    }
    return add;
}
bool TMTape3D::moveTapeHead(const TMTapeDirection &direction) {
    TMTape2D &tape = (*this)[currentIndex];
    int add = 0;
    const bool &moved = tape.moveTapeHead(direction);
    if(!moved) {
        add = (direction == Back) ? 1 : (direction == Front) ? -1 : 0;
        currentIndex += add;
        (*this)[currentIndex];
    }
    for(const auto & currentTape : cells) {
        currentTape->currentIndex = tape.currentIndex;
    }
    return add;
}

void TMTape1D::print() const {
    int i=static_cast<int>(-(cells.size()/2));
    for(const auto &currentCell : cells) {
        if(i == currentIndex) std::cout << "\x1B[31m";
        std::cout << currentCell->symbol << "\033[0m ";
       i++;
    }
    std::cout << std::endl << std::endl;
}
void TMTape2D::print() const {
    const int greatestSize = TMTapeUtils::getGreatestSize(cells);
    int j = static_cast<int>(-(cells.size()/2));
    for (const auto& currentCellRow : cells) {
        for(int i=-greatestSize/2;i<=greatestSize/2;i++) {
            if(j == currentIndex && i == currentCellRow->currentIndex) {
                std::cout << "\x1B[31m";
            }
            std::cout << (*currentCellRow)[i].symbol << "\033[0m ";
        }
        j++;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void TMTape3D::print() const {
    for (const auto& currentCell2D : cells) {
        currentCell2D->print();
    }
    std::cout << std::endl;
}

