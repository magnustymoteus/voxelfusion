//

#ifndef VOXELFUSION_TMTAPEUTILS_H
#define VOXELFUSION_TMTAPEUTILS_H

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "invariants.h"

namespace TMTapeUtils {
    int translateIndex(const int &size, const int &index);
    template<class TMTapeElement>
    void expandTape(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int addedSize) {
        PRECONDITION(cells.size() % 2 == 1);
        for(unsigned int i=0;i<addedSize;i++) {
            cells.insert(cells.begin(), std::make_shared<TMTapeElement>());
            cells.push_back(std::make_shared<TMTapeElement>());
        }
        POSTCONDITION(cells.size() % 2 == 1); // the size must always be odd
    }

    template<class TMTapeElement>
    TMTapeElement getTapeElementNoExpand(const std::vector<std::shared_ptr<TMTapeElement>> &cells, const int index) {
        PRECONDITION(cells.size() % 2 == 1);
        const int size = cells.size();
        if(-size/2 > index || size/2 < index)
            return TMTapeElement();
        POSTCONDITION(cells.size() % 2 == 1);
        return *cells[translateIndex(cells.size(), index)];
    }

    template<class TMTapeElement>
    TMTapeElement& getTapeElement(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int index) {
        PRECONDITION(cells.size() % 2 == 1);
        const int size = (cells.size());
        if(-size/2 > index || size/2 < index) {
            const int amountToInsert = std::abs(index) - (size / 2);
            expandTape(cells, amountToInsert);
        }
        POSTCONDITION(cells.size() % 2 == 1);
        return *cells[translateIndex(cells.size(), index)];
    }
    template<class TMTapeElement>
    int getGreatestSize(const std::vector<std::shared_ptr<TMTapeElement>> &cells) {
        PRECONDITION(cells.size() % 2 == 1);
        const int greatestSize = static_cast<int>((*std::max_element(cells.begin(), cells.end(),
                                                                     [](const auto &a, const auto &b) {
                                                                         return a->cells.size() < b->cells.size();
                                                                     }))->cells.size());
        POSTCONDITION(cells.size() % 2 == 1);
        return greatestSize;
    }

};


#endif //VOXELFUSION_TMTAPEUTILS_H
