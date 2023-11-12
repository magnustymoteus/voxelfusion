//
// Created by gruzi on 30/09/2023.
//
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "json.hpp"

#include "utils.h"

#include "CFG.h"

using nlohmann::json;
using namespace CFGUtils;


bool hasConflicts(const std::set<std::string> &variables, const std::set<std::string> &terminals) {
    for(const std::string &currentVariable : variables) {
        if(terminals.find(currentVariable) != terminals.end()) return true;
    }
    return false;
}
bool CFG::isTerminal(const std::string &symbol) const {
    return terminals.find(symbol) != terminals.end();
}

void CFG::ll() const {
    LL1Parser{*this}.print();
}
CFG::CFG(const std::set<std::string> &variables_arg,
         const std::set<std::string> &terminals_arg,
         std::map<std::string, CFGProductionBodies> &production_rules_arg,
         const std::string &starting_variable_arg) :

         variables(variables_arg), terminals(terminals_arg), production_rules(production_rules_arg),
         starting_variable(starting_variable_arg){

    std::string errorMessage;
    if(!isValid(errorMessage)) throw std::invalid_argument(errorMessage.c_str());
}
CFG::CFG(const std::string &jsonPath){
    std::ifstream input(jsonPath);
    json j = json::parse(input);

    variables = j["Variables"];
    terminals = j["Terminals"];
    const json &productions = j["Productions"];
    for(const json &currentProduction : productions) {
        const std::string &head = currentProduction["head"];
        CFGProductionBody body = currentProduction["body"];
        if(body.empty()) body.insert(body.end(), "");
        production_rules[head].insert(production_rules[head].end(), body);
    }
    starting_variable = j["Start"];


    std::string errorMessage;
    if(!isValid(errorMessage)) throw std::invalid_argument(errorMessage.c_str());
}
bool CFG::isValid(std::string &errorMessageRef) const {
    std::string errorMessage;
    if(variables.find(starting_variable) == variables.end()) {
        errorMessage = "Starting variable is not a member of the variable set";
    }
    if(hasConflicts(variables, terminals)) {
        errorMessage="Intersection of variables and terminals is not empty!";
    }
    for(const std::pair<std::string, CFGProductionBodies> currentRule : production_rules) {
        if(variables.find(currentRule.first) == variables.end()) {
            errorMessage = "Head of a rule is not in variables set!";
        }
        for(const CFGProductionBody &currentBody : currentRule.second) {
            for(const std::string &currentString : currentBody) {
                if(!currentString.empty() && (variables.find(currentString) == variables.end() &&
                   terminals.find(currentString) == terminals.end())) {
                    errorMessage = "A string in rule body contains an invalid symbol '"+currentString+"' !";
                }
            }
        }
    }
    if(!errorMessage.empty()) errorMessageRef = errorMessage;
    return errorMessage.empty();
}
std::string CFG::bodyToStr(const std::vector<std::string> &body) {
    std::string result;
    if(!body.empty()) {
        result += "`";
        for (int i = 0; i < body.size(); i++) {
            if (i > 0) result += " ";
            result += body[i];
        }
        result += "`";
    }
    return result;
}
void CFG::print() const {
    std::cout << "V = ";
    printSet(getVariables());

    std::cout << "T = ";
    printSet(getTerminals());

    std::cout << "P = {\n";
    for (std::pair<std::string, CFGProductionBodies> currentRule: production_rules) {
        sort(currentRule.second.begin(), currentRule.second.end());
        for (int i=0; i < currentRule.second.size(); i++) {
            std::cout << "\t" << currentRule.first << " -> `";
            for (int j=0; j < currentRule.second[i].size(); j++) {
                std::cout << currentRule.second[i][j];
                if(j+1 != currentRule.second[i].size()) std::cout << " ";
            }
            std::cout << "`\n";
        }
    }
    std::cout << "}\n";
    std::cout << "S = " << starting_variable;
    std::cout << std::endl;
}

std::set<std::string> CFG::getVariables() const {return variables;}
std::set<std::string> CFG::getTerminals() const {return terminals;}
CFGProductionRules  CFG::getProductionRules() const {return production_rules;}
std::string CFG::getStartingVariable() const {return starting_variable;}
CFGProductionBodies CFG::getProductionBodies(const std::string &productionHead) const {
    return production_rules.at(productionHead);
}

std::set<std::string> CFG::computeFirstSet(const std::string &variable) const {
    std::set<std::string> result;
    if(isTerminal(variable) || variable.empty())
        return {variable};
    for(const CFGProductionBody &currentBody : getProductionBodies(variable)) {
        const std::set<std::string> &set = computeFirstSet(currentBody[0]);
        if(set.find("") != set.end() && currentBody.size() > 1) {
            const std::set<std::string> &nextSet = computeFirstSet(currentBody[1]);
            result.insert(nextSet.begin(), nextSet.end());
        }
        result.insert(set.begin(), set.end());
    }
    return result;
}


std::map<std::string, std::set<std::string>> CFG::computeFirstSets() const {
    std::map<std::string, std::set<std::string>> result;
    for(const std::string &currentVariable : getVariables()) {
        result[currentVariable] = computeFirstSet(currentVariable);
    }
    return result;
}

void CFG::computeFollowSet(const std::string &variable,
                                        std::map<std::string,std::set<std::string>> &followSets,
                                        bool &setHasChanged) const {
    // TO DO: fix this by replacing FIRST(...) - EPSILON U FOLLOW(...) TO FIRST(...) - EPSILOJ U FIRST(...) because
    // when epsilon gets derived u need to get the first of the next one

    /* FOLLOW(A) = {a|S ⇒* αAaβ where α, β can be any strings} */
    if(variable == getStartingVariable())
        insertIfNotASubset(followSets[variable],{EOS_MARKER}, setHasChanged);
    if(production_rules.find(variable) != production_rules.end()) {
        for (const CFGProductionBody &currentBody: getProductionBodies(variable)) {
            for (int i = 0; i < currentBody.size(); i++) {
                const std::string &current = currentBody[i];
                const bool &currentIsTerminal = isTerminal(current);
                if (i != currentBody.size() - 1 && !currentIsTerminal) {
                    bool hasEpsilon = false;
                    for (int j = i + 1; j < currentBody.size(); j++) {
                        const std::string &currentNext = currentBody[j];
                        std::set<std::string> firstSet = computeFirstSet(currentNext);
                        hasEpsilon = firstSet.find("") != firstSet.end();
                        firstSet.erase("");
                        insertIfNotASubset(followSets[current], firstSet, setHasChanged);
                        if (!hasEpsilon) break;
                    }
                    if (hasEpsilon) {
                        insertIfNotASubset(followSets[current], followSets[variable], setHasChanged);
                    }

                }
                    /* If production is of form A → αB, then Follow (B) ={FOLLOW (A)}. */
                else if (!currentIsTerminal && !current.empty()) {
                    const std::set<std::string> &followSet = followSets[variable];
                    insertIfNotASubset(followSets[current], followSet, setHasChanged);
                }
            }
        }
    }
}


std::map<std::string, std::set<std::string>> CFG::computeFollowSets() const {
    // update follow sets until no update is happening
    std::map<std::string, std::set<std::string>> result;
    bool setHasChanged = true;
    while(setHasChanged) {
        setHasChanged = false;
        for (const std::string &currentVariable: getVariables()) {
            computeFollowSet(currentVariable, result, setHasChanged);
        }
    }
    for(auto &currentFollowSet : result) {
        currentFollowSet.second.erase("");
    }
    return result;
}
