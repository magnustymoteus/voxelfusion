//
// Created by gruzi on 02/11/2023.
//

#ifndef MTMDTURINGMACHINE_TMTAPE_H
#define MTMDTURINGMACHINE_TMTAPE_H

#include <vector>
#include "TMTapeCell.h"
#include <memory>

enum TMTapeDirection {Left='L',Right='R',Up='U',Down='D',Front='F',Back='B',Stationary='S'};

#define SYMBOL_ANY "ANY"

/**
 * @brief A functor that outputs a direction depending on their probability
 */
class TMTapeProbabilisticDirection {
public:

    const std::vector<TMTapeDirection> directions;
    const std::vector<float> probabilities;

    TMTapeProbabilisticDirection(const std::vector<TMTapeDirection> &directions,
                                 const std::vector<float> &probabilities);
    explicit TMTapeProbabilisticDirection(const TMTapeDirection &direction)
    : directions({direction}), probabilities({1.0f}) {}

    TMTapeDirection operator()() const;
};


class TMTape {
protected:
public:

    virtual ~TMTape() = default;
    virtual std::string getCurrentSymbol() const = 0;
    virtual bool moveTapeHead(const TMTapeDirection &direction) = 0;
    virtual void replaceCurrentSymbol(const std::string &newSymbol) = 0;
    virtual unsigned int getElementSize() const = 0;

    int currentIndex;
    int zeroAnchor;

    TMTape() : currentIndex(0), zeroAnchor(0) {}
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

    const std::vector<std::shared_ptr<TMTapeCell>> &getCells() const;

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

    const std::vector<std::shared_ptr<TMTape1D>> &getCells() const;

};
class TMTape3D final : public TMTape {
    std::vector<std::shared_ptr<TMTape2D>> cells;
public:

    TMTape3D() : TMTape(), cells({std::make_shared<TMTape2D>()}) {}
    ~TMTape3D() final = default;

    std::string getCurrentSymbol() const final;
    bool moveTapeHead(const TMTapeDirection &direction) final;
    void replaceCurrentSymbol(const std::string &newSymbol) final;
    unsigned int getElementSize() const final;

    void print() const;

    TMTape2D& operator[](const signed int &index);
    TMTape2D at(const signed int &index) const;
    const std::vector<std::shared_ptr<TMTape2D>>& getCells();

};
typedef std::vector<TMTape*> TMTapes;
#endif //MTMDTURINGMACHINE_TMTAPE_H
