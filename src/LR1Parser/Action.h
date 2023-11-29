//

#ifndef VOXELFUSION_ACTION_H
#define VOXELFUSION_ACTION_H

#include "CFG/AugmentedCFG.h"
#include "LR1ParsingSpace.h"

#include "json.hpp"

class LR1ParseTableEntry;

// LR parsing actions using functors
class Action {
public:
    virtual ~Action() = default;
    void print() const;
    virtual std::string getString() const = 0;
    virtual void operator()(LR1ParsingSpace &parsingSpace,
            const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const = 0;
    virtual nlohmann::json getJson() const = 0;
};

class Reduce : public Action {
public:
    const CFGProductionBody body;
    const std::string head;
    void operator()(LR1ParsingSpace &parsingSpace,
            const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const final;
    std::string getString() const final;
    nlohmann::json getJson() const final;
    explicit Reduce(const std::string &head, const CFGProductionBody &body) : head(head), body(body) {}
};

class Shift : public Action {
public:
    const unsigned int index;
    void operator()(LR1ParsingSpace &parsingSpace,
            const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const final;
    std::string getString() const final;
    nlohmann::json getJson() const final;
    explicit Shift(const unsigned int &index) : index(index) {}
};

class Accept : public Action {
public:
    void operator()(LR1ParsingSpace &parsingSpace,
            const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const final;
    std::string getString() const final;
    nlohmann::json getJson() const final;
};

#endif //VOXELFUSION_ACTION_H
