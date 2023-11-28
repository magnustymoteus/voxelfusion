//

#ifndef VOXELFUSION_ACTION_H
#define VOXELFUSION_ACTION_H

#include "CFG/AugmentedCFG.h"
#include "LR1ParsingSpace.h"

// LR parsing actions using functors

class Action {
public:
    virtual ~Action() = default;
    void print() const;
    virtual std::string getString() const = 0;
    virtual void operator()(LR1ParsingSpace &parsingSpace) const = 0;
};

class Reduce : public Action {
public:
    const AugmentedProductionBody body;
    const std::string head;
    void operator()(LR1ParsingSpace &parsingSpace) const final;
    std::string getString() const final;
    explicit Reduce(const std::string &head, const AugmentedProductionBody &body) : head(head), body(body) {}
};

class Shift : public Action {
public:
    const unsigned int index;
    void operator()(LR1ParsingSpace &parsingSpace) const final;
    std::string getString() const final;
    explicit Shift(const unsigned int &index) : index(index) {}
};

class Accept : public Action {
public:
    void operator()(LR1ParsingSpace &parsingSpace) const final;
    std::string getString() const final;
};

#endif //VOXELFUSION_ACTION_H
