//
// Created by gruzi on 02/11/2023.
//

#include "TMTape.h"

#include <iostream>
#include <algorithm>

int translateIndex(const unsigned long &size, const unsigned int &index) {
    return static_cast<int>(size/2+index);
}
template<class TMTapeElement>
void expandTape(std::vector<TMTapeElement> &cells, const unsigned long &addedSize) {
    cells.insert(cells.begin(), addedSize, TMTapeElement());
    cells.insert(cells.end(), addedSize, TMTapeElement());
}

template<class TMTapeElement>
TMTapeElement getTapeElementNoExpand(const std::vector<TMTapeElement> &cells, const signed long &index) {
    const long size = static_cast<long> (cells.size());
    if(-size/2 > index || size/2 < index)
        return TMTapeElement();
    return cells[translateIndex(cells.size(), index)];
}
template<class TMTapeElement>
TMTapeElement& getTapeElement(std::vector<TMTapeElement> &cells, const signed long &index) {
    const long size = static_cast<long> (cells.size());
    if(-size/2 > index || size/2 < index) {
        const unsigned long amountToInsert = std::abs(index) - (size / 2);
        expandTape(cells, amountToInsert);
    }
    return cells[translateIndex(cells.size(), index)];
}


TMTapeCell & TMTape1D::operator[](const long &index) {return getTapeElement(cells, index);}
TMTape1D & TMTape2D::operator[](const long &index) {return getTapeElement(cells, index);}
TMTape2D &TMTape3D::operator[](const long &index) {return getTapeElement(cells, index);}

TMTapeCell TMTape1D::at(const long &index) const {return getTapeElementNoExpand(cells, index);}
TMTape1D TMTape2D::at(const long &index) const {return getTapeElementNoExpand(cells, index);}
TMTape2D TMTape3D::at(const long &index) const {return getTapeElementNoExpand(cells, index);}


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
    for(TMTape1D& currentTape : cells) {
        currentTape.currentIndex = tape.currentIndex;
    }
    return add;
}
bool TMTape3D::moveTapeHead(const TMTapeDirection &direction) {
    TMTape2D &tape = (*this)[currentIndex];
    const bool &moved = tape.moveTapeHead(direction);
    int add = 0;
    if(!moved) {
        add = (direction == Back) ? 1 : (direction == Front) ? -1 : 0;
        currentIndex += add;
        (*this)[currentIndex];
    }
    for(TMTape2D& currentTape : cells) {
        currentTape.currentIndex = tape.currentIndex;
    }
    return add;
}

void TMTape1D::print() const {
    long i=static_cast<long>(-(cells.size()/2));
    for(const TMTapeCell &currentCell : cells) {
        if(i == currentIndex) std::cout << "\x1B[31m";
        std::cout << currentCell.symbol << "\033[0m ";
        i++;
    }
    std::cout << std::endl;
}
void TMTape2D::print() const {
    const long greatestSize = static_cast<long>(std::max_element(cells.begin(), cells.end(),
                                                        [](const TMTape1D &a, const TMTape1D &b) {
        return a.cells.size() < b.cells.size();
    })->cells.size());
    long j = static_cast<long>(-(cells.size()/2));
    for (TMTape1D currentCellRow : cells) {
        for(long i=-greatestSize/2;i<=greatestSize/2;i++) {
            if(j == currentIndex && i == currentCellRow.currentIndex) std::cout << "\x1B[31m";
            std::cout << currentCellRow[i].symbol << "\033[0m ";
        }
        j++;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}