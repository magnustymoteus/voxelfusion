//

#ifndef VOXELFUSION_SYMBOLTRIE_H
#define VOXELFUSION_SYMBOLTRIE_H

#include "MTMDTuringMachine/TMTape.h"
#include <unordered_map>
#include <optional>


template<typename ValueType>
class TrieNode {
public:
    std::unordered_map<std::string, std::shared_ptr<TrieNode<ValueType>>> children;
    std::optional<ValueType> value;

    TrieNode() = default;
};

template<typename ValueType>
class Trie {
public:
    std::shared_ptr<TrieNode<ValueType>> root;

    Trie() {
        root = std::make_shared<TrieNode<ValueType>>();
    }

    void insert(const std::vector<std::string>& sequence, const ValueType& value) {
        std::shared_ptr<TrieNode<ValueType>>* node = &root;
        for (const std::string &c : sequence) {
            if ((*node)->children.find(c) == (*node)->children.end()) {
                (*node)->children[c] = std::make_shared<TrieNode<ValueType>>();
            }
            node = &(*node)->children[c];
        }
        (*node)->value.emplace(value);
    }

    std::optional<ValueType> search(const std::vector<std::string>& sequence) const {
        const std::shared_ptr<TrieNode<ValueType>>* node = &root;
        for (const std::string &c : sequence) {
            if ((*node)->children.find(c) == (*node)->children.end()) {
                if((*node)->children.find(SYMBOL_ANY) == (*node)->children.end()) {
                    return std::nullopt;
                }
                node = &(*node)->children[SYMBOL_ANY];
            }
            else node = &(*node)->children[c];
        }
        return (*node)->value;
    }
};



#endif //VOXELFUSION_SYMBOLTRIE_H
