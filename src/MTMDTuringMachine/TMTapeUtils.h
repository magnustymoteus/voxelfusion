//

#ifndef VOXELFUSION_TMTAPEUTILS_H
#define VOXELFUSION_TMTAPEUTILS_H

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "invariants.h"
#include <mutex>

namespace TMTapeUtils {
    inline std::mutex expansionMutex;

    template<class TMTapeElement>
    TMTapeElement getTapeElementNoExpand(const std::vector<std::shared_ptr<TMTapeElement>> &cells, const int &index, const int &zeroAnchor) {
        const int upperBound = cells.size()-zeroAnchor-1;
        if((index >= 0 && upperBound < index) || (-zeroAnchor > index)) {
            return TMTapeElement();
        }
        return *cells[index+zeroAnchor];
    }
    template<class TMTapeElement>
    int getMaximumIndex(const std::vector<std::shared_ptr<TMTapeElement>> &cells, const int &zeroAnchor) {
        return cells.size()-zeroAnchor-1;
    }

    template<class TMTapeElement>
    TMTapeElement& getTapeElement(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int &index, int &zeroAnchor) {
        const int upperBound = cells.size()-zeroAnchor-1;
        if(index >= 0 && upperBound < index) {
            expansionMutex.lock();
            for(unsigned int i=0;i<index-upperBound;i++) {
                cells.insert(cells.end(), std::make_shared<TMTapeElement>());
            }
            expansionMutex.unlock();
        }
        else if(-zeroAnchor > index) {
            expansionMutex.lock();
            for(unsigned int i=0;i<(-zeroAnchor-index);i++) {
                cells.insert(cells.begin(), std::make_shared<TMTapeElement>());
            }
            zeroAnchor = -index;
            expansionMutex.unlock();
        }
        return *cells[index+zeroAnchor];
    }
    template<class TMTapeElement>
    int getGreatestSize(const std::vector<std::shared_ptr<TMTapeElement>> &cells) {
        const int greatestSize = static_cast<int>((*std::max_element(cells.begin(), cells.end(),
                                                                     [](const auto &a, const auto &b) {
                                                                         return a->cells.size() < b->cells.size();
                                                                     }))->cells.size());
        return greatestSize;
    }
    template<class TMTapeElement>
            void setIndexForAllCells(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int index) {
        for (const auto &cell: cells) {
            cell->currentIndex = index;
        }
    }
};


#endif //VOXELFUSION_TMTAPEUTILS_H
