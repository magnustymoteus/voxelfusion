//

#include "SyntaxTree.h"
#include <graphviz/gvc.h>
#include <iostream>

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

void visualizeNode(const STNode &node, Agraph_t* &graph) {
    static int nodeCounter = 0;

    char* parentLabel = strdup(getLabelOfNode(node).data());
    std::string parentId = std::to_string(nodeCounter);
    Agnode_t *parentNode = agnode(graph, parentId.data(), 1);

    agset(parentNode, "label", parentLabel);

    nodeCounter++;
    for(auto iter = node.children.rbegin(); iter != node.children.rend(); iter++) {
        const std::shared_ptr<STNode>& currentChild = *iter;
        char* childLabel = strdup(getLabelOfNode(*currentChild).data());
        std::string childId = std::to_string(nodeCounter);

        Agnode_t *visualizedChild = agnode(graph, childId.data(), 1);

        agset(visualizedChild, "label", childLabel);
        agedge(graph, parentNode, visualizedChild, nullptr, 1);

        visualizeNode(*currentChild, graph);
    }
    if(node.children.empty()) agset(parentNode, "fillcolor", "lightgreen");
}
void STNode::exportVisualization(const std::string &fileName) const {
    GVC_t *gvc;
    std::string title = "Syntax Tree";
    Agraph_t* graph = agopen(title.data(), Agundirected, nullptr);

    agattr(graph, AGNODE, "label", "");
    agattr(graph, AGNODE, "style", "filled");
    agattr(graph, AGNODE, "fillcolor", "lightblue");
    agattr(graph, AGNODE,"shape","box");

    visualizeNode(*this, graph);

    gvc = gvContext();
    gvLayout(gvc, graph, "dot");
    gvRenderFilename(gvc, graph, "png", fileName.c_str());

    gvFreeLayout(gvc, graph);
    agclose(graph);
    gvFreeContext(gvc);
}