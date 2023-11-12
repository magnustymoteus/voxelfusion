//
// Created by gruzi on 30/09/2023.
//

#ifndef CFG_CFG_H
#define CFG_CFG_H

#include <set>
#include <vector>
#include <map>
#include <string>
#include "LL1Parser.h"

#define EOS_MARKER "<EOS>"

typedef std::vector<std::string> CFGProductionBody;
typedef std::vector<CFGProductionBody> CFGProductionBodies;
typedef std::map<std::string, CFGProductionBodies> CFGProductionRules;
typedef std::pair<std::string, CFGProductionBodies> CFGProductionRule;


class CFG {
private:
    bool isValid(std::string &errorMessage) const;
    std::set<std::string> variables;
    std::set<std::string> terminals;
    CFGProductionRules production_rules; // key: head : array of bodies
    std::string starting_variable;

    void computeFollowSet(
            const std::string &variable, std::map<std::string, std::set<std::string>> &followSets,
            bool &setHasChanged) const;

public:
    CFG(const std::set<std::string> &variables_arg,
        const std::set<std::string> &terminals_arg,
        CFGProductionRules &production_rules_arg,
        const std::string &starting_variable_arg);
    CFG() = default;
    explicit CFG(const std::string &jsonPath);

    void print() const;

    void ll() const;
    [[nodiscard]] bool isTerminal(const std::string &symbol) const;

    [[nodiscard]] std::set<std::string> getVariables() const;
    [[nodiscard]] std::set<std::string> getTerminals() const;
    [[nodiscard]] CFGProductionRules  getProductionRules() const;
    [[nodiscard]] std::string getStartingVariable() const;
    [[nodiscard]] CFGProductionBodies getProductionBodies(const std::string &productionHead) const;

    [[nodiscard]] std::set<std::string> computeFirstSet(const std::string &variable) const;

    [[nodiscard]] std::map<std::string, std::set<std::string>> computeFirstSets() const;

    [[nodiscard]] std::map<std::string, std::set<std::string>> computeFollowSets() const;

    [[nodiscard]] static std::string bodyToStr(const std::vector<std::string> &body);

};


#endif //CFG_CFG_H
