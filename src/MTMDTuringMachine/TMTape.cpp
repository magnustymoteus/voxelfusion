//
// Created by gruzi on 02/11/2023.
//

#include "TMTape.h"
#include "lib/invariants.h"
#include "MTMDTuringMachine/TMTapeUtils.h"

#include <iostream>
#include <random>


TMTapeCell & TMTape1D::operator[](const int &index) {
    return TMTapeUtils::getTapeElement(cells, index, zeroAnchor);
}
TMTape1D & TMTape2D::operator[](const int &index) {
    return TMTapeUtils::getTapeElement(cells, index, zeroAnchor);
}
TMTape2D& TMTape3D::operator[](const int &index) {
    return TMTapeUtils::getTapeElement(cells, index, zeroAnchor);
}

TMTapeCell TMTape1D::at(const int &index) const {
    return TMTapeUtils::getTapeElementNoExpand(cells, index, zeroAnchor);
}
TMTape1D TMTape2D::at(const int &index) const {
    return TMTapeUtils::getTapeElementNoExpand(cells, index, zeroAnchor);
}
TMTape2D TMTape3D::at(const int &index) const {
    return TMTapeUtils::getTapeElementNoExpand(cells, index, zeroAnchor);
}

const std::vector<std::shared_ptr<TMTape2D>> &TMTape3D::getCells() {
    return cells;
}

unsigned int TMTape1D::getElementSize() const {
    return cells.size();
}
unsigned int TMTape2D::getElementSize() const {
    return TMTapeUtils::getGreatestSize(cells);
}
unsigned int TMTape3D::getElementSize() const {
    return TMTapeUtils::getGreatestSize(cells);
}

void TMTape1D::replaceCurrentSymbol(const std::string &newSymbol) {
    if(newSymbol != SYMBOL_ANY) (*this)[currentIndex].symbol = newSymbol;
}
void TMTape2D::replaceCurrentSymbol(const std::string &newSymbol) {
    if(newSymbol != SYMBOL_ANY) (*this)[currentIndex].replaceCurrentSymbol(newSymbol);
}
void TMTape3D::replaceCurrentSymbol(const std::string &newSymbol) {
    if(newSymbol != SYMBOL_ANY) (*this)[currentIndex].replaceCurrentSymbol(newSymbol);
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

// the next three methods are ugly...
bool TMTape1D::moveTapeHead(const TMTapeDirection &direction) {
    int add = 0;
    switch(direction) {
        case Right:
            add = 1;
            break;
        case Left:
            add = -1;
            break;
        default:
            break;
    }
    currentIndex += add;
    (*this)[currentIndex];
    return add || direction == Stationary;
}
bool TMTape2D::moveTapeHead(const TMTapeDirection &direction) {
    int add = (direction == Down) ? -1 : (direction == Up) ? 1 : 0;
    const int subIndex = at(currentIndex).currentIndex;

    currentIndex += add;
    (*this)[currentIndex];

    TMTapeUtils::setIndexForAllCells<TMTape1D>(cells, subIndex);
    for(const auto & currentTape : cells) {
        currentTape->moveTapeHead(direction);
    }
    return add;
}
bool TMTape3D::moveTapeHead(const TMTapeDirection &direction) {
    int add = (direction == Back) ? -1 : (direction == Front) ? 1 : 0;
    const int subIndex = at(currentIndex).currentIndex;
    const int subSubIndex = at(currentIndex).at(subIndex).currentIndex;

    currentIndex += add;
    (*this)[currentIndex];

    TMTapeUtils::setIndexForAllCells<TMTape2D>(cells, subIndex);
    for(const auto &currentTape : cells) {
        if(add) TMTapeUtils::setIndexForAllCells<TMTape1D>(currentTape->cells, subSubIndex);
        else currentTape->moveTapeHead(direction);
    }
    return add;
}


void TMTape1D::print() const {
    int i= -zeroAnchor;
    for(const auto &currentCell : cells) {
        if(i == currentIndex) std::cout << "\x1B[31m";
        std::cout << currentCell->symbol << "\033[0m ";
       i++;
    }
    std::cout << std::endl << std::endl;
}
void TMTape2D::print() const {
    const int greatestSize = TMTapeUtils::getGreatestSize(cells);
    int j = -zeroAnchor;
    for (const auto& currentCellRow : cells) {
        for(int i=-currentCellRow->zeroAnchor;i<greatestSize;i++) {
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

const std::vector<std::shared_ptr<TMTape1D>> &TMTape2D::getCells() const {
    return cells;
}

void TMTape3D::print() const {
    int x = -zeroAnchor;
    for (const auto &currentCell2D: cells) {
        const int greatestSize = TMTapeUtils::getGreatestSize(currentCell2D->cells);
        int j = -currentCell2D->zeroAnchor;
        for (const auto &currentCellRow: currentCell2D->cells) {
            for (int i = -currentCellRow->zeroAnchor; i < greatestSize; i++) {
                if (x == currentIndex && j == currentCell2D->currentIndex && i == currentCellRow->currentIndex) {
                    std::cout << "\x1B[31m";
                }
                std::cout << (*currentCellRow)[i].symbol << "\033[0m ";
            }
            j++;
            std::cout << std::endl;
        }
        x++;
        std::cout << std::endl;
    }
}

TMTapeProbabilisticDirection::TMTapeProbabilisticDirection(const std::vector<TMTapeDirection> &directions,
                                                           const std::vector<float> &probabilities)
                                                           : directions(directions), probabilities(probabilities)
                                                           {
    assert(directions.size() == probabilities.size());
                                                           }

TMTapeDirection TMTapeProbabilisticDirection::operator()() const {
    std::mt19937 gen(std::random_device{}());
    std::discrete_distribution<std::size_t> index{probabilities.begin(), probabilities.end()};
    return directions[index(gen)];
}