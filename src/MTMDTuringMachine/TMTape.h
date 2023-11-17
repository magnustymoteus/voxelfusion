//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_TMTAPE_H
#define MTMDTURINGMACHINE_TMTAPE_H

#include <vector>
#include "TMTapeCell.h"

enum TMTapeDirection {Left='L',Right='R',Up='U',Down='D',Front='F',Back='B',Stationary='S'};

class TMTape {
public:
    virtual ~TMTape() = default;
    virtual std::string getCurrentSymbol() const = 0;
    virtual bool moveTapeHead(const TMTapeDirection &direction) = 0;
    virtual void replaceCurrentSymbol(const std::string &newSymbol) = 0;
};
class TMTape1D final : public TMTape {
public:
    std::vector<TMTapeCell> cells;
    signed long currentIndex;

    TMTape1D() : cells({TMTapeCell()}), currentIndex(0) {}

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    void print() const;

    TMTapeCell& operator[](const signed long &index);
};
class TMTape2D final : public TMTape {
public:
    std::vector<TMTape1D> cells;
    signed long currentIndex;

    TMTape2D() : cells({TMTape1D()}), currentIndex(0) {}

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    void print() const;


    TMTape1D& operator[](const signed long &index);
};
class TMTape3D final : public TMTape {
public:
    std::vector<TMTape2D> cells;
    signed long currentIndex;

    TMTape3D() : cells({TMTape2D()}), currentIndex(0) {}

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;

    TMTape2D& operator[](const signed long &index);
};
typedef std::vector<TMTape*> TMTapes;
#endif //MTMDTURINGMACHINE_TMTAPE_H
