//

#ifndef VOXELFUSION_TMTAPEUTILS_H
#define VOXELFUSION_TMTAPEUTILS_H

#include <vector>
#include <memory>

namespace TMTapeUtils {
    int translateIndex(const int &size, const int &index);
    template<class TMTapeElement>
    void expandTape(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int addedSize) {
        cells.insert(cells.begin(), addedSize, std::make_shared<TMTapeElement>());
        cells.insert(cells.end(), addedSize, std::make_shared<TMTapeElement>());
    }

    template<class TMTapeElement>
    TMTapeElement getTapeElementNoExpand(const std::vector<std::shared_ptr<TMTapeElement>> &cells, const int index) {
        const int size = cells.size();
        if(-size/2 > index || size/2 < index)
            return TMTapeElement();
        return *cells[translateIndex(cells.size(), index)];
    }
    template<class TMTapeElement>
    TMTapeElement& getTapeElement(std::vector<std::shared_ptr<TMTapeElement>> &cells, const int index) {
        const int size = (cells.size());
        if(-size/2 > index || size/2 < index) {
            const int amountToInsert = std::abs(index) - (size / 2);
            expandTape(cells, amountToInsert);
        }
        return *cells[translateIndex(cells.size(), index)];
    }
    template<class TMTapeElement>
    int getGreatestSize(const std::vector<std::shared_ptr<TMTapeElement>> &cells) {
        const int greatestSize = static_cast<int>((*std::max_element(cells.begin(), cells.end(),
                                                                     [](const auto &a, const auto &b) {
                                                                         return a->cells.size() < b->cells.size();
                                                                     }))->cells.size());
        return greatestSize;
    }

};


#endif //VOXELFUSION_TMTAPEUTILS_H
