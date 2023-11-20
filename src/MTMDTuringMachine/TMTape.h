//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_TMTAPE_H
#define MTMDTURINGMACHINE_TMTAPE_H

#include <vector>
#include "TMTapeCell.h"
#include <memory>

enum TMTapeDirection {Left='L',Right='R',Up='U',Down='D',Front='F',Back='B',Stationary='S'};

class TMTape {
public:
    virtual ~TMTape() = default;
    virtual std::string getCurrentSymbol() const = 0;
    virtual bool moveTapeHead(const TMTapeDirection &direction) = 0;
    virtual void replaceCurrentSymbol(const std::string &newSymbol) = 0;
    virtual unsigned int getElementSize() const = 0;

    int currentIndex;

    TMTape() : currentIndex(0) {}
};
class TMTape1D final : public TMTape {
public:
    std::vector<std::shared_ptr<TMTapeCell>> cells;

    TMTape1D() : TMTape(), cells({std::make_shared<TMTapeCell>()}) {}
    ~TMTape1D() final = default;

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    unsigned int getElementSize() const final;
    void print() const;

    TMTapeCell& operator[](const signed int &index);
    TMTapeCell at(const signed int &index) const;
};
class TMTape2D final : public TMTape {
public:
    std::vector<std::shared_ptr<TMTape1D>> cells;

    TMTape2D() : TMTape(), cells({std::make_shared<TMTape1D>()}) {}
    ~TMTape2D() final = default;

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    unsigned int getElementSize() const final;
    void print() const;


    TMTape1D& operator[](const signed int &index);
    TMTape1D at(const signed int &index) const;
};
class TMTape3D final : public TMTape {
public:
    std::vector<std::shared_ptr<TMTape2D>> cells;

    TMTape3D() : TMTape(), cells({std::make_shared<TMTape2D>()}) {}
    ~TMTape3D() final = default;

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    unsigned int getElementSize() const final;

    TMTape2D& operator[](const signed int &index);
    TMTape2D at(const signed int &index) const;
};
typedef std::vector<TMTape*> TMTapes;
#endif //MTMDTURINGMACHINE_TMTAPE_H
