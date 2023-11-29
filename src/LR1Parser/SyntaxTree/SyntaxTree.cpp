//

#include "SyntaxTree.h"
#include <fstream>

std::string getLabelOfNode(const STNode &node) {
    std::string label = node.label;
    const auto &token = node.getToken();
    if(token) {
        const auto &findTerminal = TokenMapping::terminals.find(token->type);
        std::string type;
        if(findTerminal != TokenMapping::terminals.end()) type = findTerminal->second;
        else type = std::to_string(token->type);
        label += "\ntype:"+type;
    }
    return label;
}

void visualizeNode(const STNode &node, std::string &currentStr) {
    static int nodeCounter = 0;
    const std::string parentLabelStr = getLabelOfNode(node);
    const std::string parentId = std::to_string(nodeCounter);

    currentStr += parentId+"[label = \""+parentLabelStr+"\"]\n";
    if(node.children.empty()) currentStr += parentId+"[fillcolor=\"lightgreen\"]\n";

    nodeCounter++;
    for(auto iter = node.children.rbegin(); iter != node.children.rend(); iter++) {
        const std::shared_ptr<STNode>& currentChild = *iter;
        const std::string childLabelStr = getLabelOfNode(*currentChild);

        currentStr += std::to_string(nodeCounter)+"[label = \""+childLabelStr+"\"]\n";
        currentStr += parentId+"--"+std::to_string(nodeCounter)+"\n";

        visualizeNode(*currentChild, currentStr);
    }
}
std::string setNodeAttribute(const std::string &attr, const std::string &value ) {
    return "node ["+attr+"=\""+value+"\"]\n";
}
void STNode::exportVisualization(const std::string &fileName) const {
    std::string resultStr = "Graph G {\nsplines=false;\n";
    resultStr += setNodeAttribute("style", "filled");
    resultStr += setNodeAttribute("fillcolor", "lightblue");
    resultStr += setNodeAttribute("shape", "box");

    visualizeNode(*this, resultStr);

    resultStr += "\n}";
    std::ofstream output(fileName);
    output << resultStr;
    output.close();
}