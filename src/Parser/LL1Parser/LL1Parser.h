//
// Created by gruzi on 22/10/2023.
//

#ifndef CFG_LL1PARSER_H
#define CFG_LL1PARSER_H

class CFG;

#include <map>
#include <string>
#include <set>

typedef std::map<std::string, std::map<std::string, std::string>> LL1ParsingTable;

class LL1Parser {
private:
    [[nodiscard]] LL1ParsingTable getParsingTable() const;
    [[nodiscard]] std::map<std::string, unsigned int> getColumnWidthsOfParsingTable(
            const LL1ParsingTable &parsingTable) const;
    void printParsingTable(const LL1ParsingTable &parsingTable,
                           const std::map<std::string, unsigned int> &columnWidths) const;
    void printHorizontalLine(const std::map<std::string,
            unsigned int> &columnWidths, const unsigned int &variableColumnWidth) const;
public:
    std::map<std::string, std::set<std::string>> followSets;
    std::map<std::string, std::set<std::string>> firstSets;

    const CFG &cfg;

    explicit LL1Parser(const CFG &cfg);
    void print() const;
};


#endif //CFG_LL1PARSER_H
