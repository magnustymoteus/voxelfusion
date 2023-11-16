//

#ifndef VOXELFUSION_TMTAPECELL_H
#define VOXELFUSION_TMTAPECELL_H

#include <memory>
#include <string>

struct TMTapeCell {
    std::string symbol;
    std::shared_ptr<TMTapeCell> left;
    std::shared_ptr<TMTapeCell> right;

    explicit TMTapeCell(const std::string &symbolArg) : symbol(symbolArg) {}
    explicit TMTapeCell(std::string &&symbolArg) : symbol(std::move(symbolArg)) {}
};

struct TMTapeCell2D : public TMTapeCell {
    std::shared_ptr<TMTapeCell2D> top;
    std::shared_ptr<TMTapeCell2D> bottom;

    explicit TMTapeCell2D(const std::string &symbolArg) : TMTapeCell(symbolArg) {}
    explicit TMTapeCell2D(std::string &&symbolArg) : TMTapeCell(std::move(symbolArg)) {}
};

struct TMTapeCell3D : public TMTapeCell {
public:
    std::shared_ptr<TMTapeCell3D> front;
    std::shared_ptr<TMTapeCell3D> back;
    std::shared_ptr<TMTapeCell3D> top;
    std::shared_ptr<TMTapeCell3D> bottom;

    explicit TMTapeCell3D(const std::string &symbolArg) : TMTapeCell(symbolArg) {}
    explicit TMTapeCell3D(std::string &&symbolArg) : TMTapeCell(std::move(symbolArg)) {}
};


#endif //VOXELFUSION_TMTAPECELL_H
