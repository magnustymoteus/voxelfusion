//

#ifndef VOXELFUSION_ACTION_H
#define VOXELFUSION_ACTION_H

#include "CFG/AugmentedCFG.h"

// LR parsing actions using functors

class Action {
public:
    virtual void operator()() = 0;
    virtual ~Action() = default;
    virtual void print() const = 0;
};

class Reduce final : public Action {
public:
    const AugmentedProductionBody body;
    const std::string head;
    void operator()() final;
    void print() const final;
    explicit Reduce(const std::string &head, const AugmentedProductionBody &body) : head(head), body(body) {}
};

class Shift final : public Action {
public:
    const unsigned int index;
    void operator()() final;
    void print() const final;
    explicit Shift(const unsigned int &index) : index(index) {}
};

class Accept final : public Action {
public:
    void operator()() final;
    void print() const final;
};

#endif //VOXELFUSION_ACTION_H
