//
// Created by gruzi on 30/09/2023.
//
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "lib/json.hpp"

#include "CFGUtils.h"

#include "CFG.h"

using nlohmann::json;


bool hasConflicts(const std::set<std::string> &variables, const std::set<std::string> &terminals) {
    for(const std::string &currentVariable : variables) {
        if(terminals.find(currentVariable) != terminals.end()) return true;
    }
    return false;
}
bool CFG::isTerminal(const std::string &symbol) const {
    return terminals.find(symbol) != terminals.end();
}

bool CFG::isVariable(const std::string &symbol) const {
    return variables.find(symbol) != variables.end();
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

    variables = j["Variables"].get<std::set<std::string>>();
    terminals = j["Terminals"].get<std::set<std::string>>();
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

std::set<std::string> CFG::getVariables() const {return variables;}

std::set<std::string> CFG::getTerminals() const {return terminals;}
std::set<std::string> CFG::getAllSymbols() const {
    std::set<std::string> allSymbols = variables;
    allSymbols.insert(terminals.begin(), terminals.end());
    return allSymbols;
}

CFGProductionRules  CFG::getProductionRules() const {return production_rules;}

std::string CFG::getStartingVariable() const {return starting_variable;}
CFGProductionBodies CFG::getProductionBodies(const std::string &productionHead) const {
    return production_rules.at(productionHead);
}
std::set<std::string> CFG::computeFirstSet(const std::string &variable) const {
    std::set<std::string> result;
    if(!isVariable(variable))
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
