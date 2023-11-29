//

#include "Action.h"
#include <iostream>
#include "CFG/CFGUtils.h"
#include "LR1Parser.h"


void Reduce::operator()(LR1ParsingSpace &parsingSpace,
        const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const {
    std::vector<std::shared_ptr<STNode>> poppedSymbols;

    for (int i = 0; i < body.size(); ++i) {
        poppedSymbols.push_back(parsingSpace.nodeStack.top());
        parsingSpace.nodeStack.pop();
        parsingSpace.stateStack.pop();
    }

    std::shared_ptr<STNode> newNode (new STNode(head, poppedSymbols));
    parsingSpace.nodeStack.push(newNode);

    const unsigned int newState = parsingSpace.stateStack.top();
    const auto &findHead = parsingTable.at(newState).gotoMap.find(head);
    if(findHead != parsingTable.at(newState).gotoMap.end()) parsingSpace.stateStack.push(findHead->second);
    else throw std::invalid_argument(
            "Cannot parse input: Cannot find goto("+std::to_string(newState)+","+head+")");
}

void Shift::operator()(LR1ParsingSpace &parsingSpace,
        const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const {
    parsingSpace.nodeStack.push(std::make_shared<STNode>(parsingSpace.input[parsingSpace.tokenIndex]));
    parsingSpace.stateStack.push(index);
    parsingSpace.tokenIndex++;
}

void Accept::operator()(LR1ParsingSpace &parsingSpace,
        const std::map<unsigned int, LR1ParseTableEntry> &parsingTable) const {
    parsingSpace.accepted = true;
}

void Action::print() const {
    std::cout << getString() << std::endl;
}

std::string Reduce::getString() const {
    return "Reduce " + head + "->" + CFGUtils::getString(body);
}

std::string Shift::getString() const {
    return "Shift " + std::to_string(index);
}

std::string Accept::getString() const {
    return "Accept";
}

using nlohmann::json;

json Reduce::getJson() const {
    json result;
    result["type"] = "Reduce";
    result["body"] = body;
    result["head"] = head;
    return result;
}

json Shift::getJson() const {
    json result;
    result["type"] = "Shift";
    result["state"] = index;
    return result;
}

json Accept::getJson() const {
    json result;
    result["type"] = "Accept";
    return result;
}