//

#ifndef VOXELFUSION_ACTION_H
#define VOXELFUSION_ACTION_H

#include "CFG/AugmentedCFG.h"

class Action {
public:
    virtual void performAction() = 0;
    virtual ~Action() = default;
};

class Reduce final : public Action {
public:
    const AugmentedProductionBody body;
    const std::string head;
    void performAction() final;
    Reduce(const std::string &head, const AugmentedProductionBody &body) : head(head), body(body) {}
};

class Shift final : public Action {
public:
    const unsigned int index;
    void performAction() final;
    Shift(const unsigned int &index) : index(index) {}
};

class Accept final : public Action {
public:
    void performAction() final;
};

#endif //VOXELFUSION_ACTION_H
