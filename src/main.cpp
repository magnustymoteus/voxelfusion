//#include "CFG/AugmentedCFG.h"
//#include "CFG/CFGUtils.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include <iostream>

using namespace std;
template<class ...TMTapeType>
void printTransition(const std::tuple<TMTapeType...> &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    std::cout << domain.state.name << " -> " << image.state.name << std::endl;
    for(const std::string &currentDomainSymbol : domain.replacedSymbols) {
        std::cout << currentDomainSymbol << " ";
    }
    std::cout << "replaced by ";
    for(const std::string &currentImageSymbol : image.replacementSymbols) {
        std::cout << currentImageSymbol << " ";
    }
    std::cout << std::endl;
    for(unsigned int i=0;i<image.directions.size();i++) {
        std::cout << "Move tapehead " << i+1 <<" to direction " << static_cast<char>(image.directions[i]) << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    auto *tape3d {new TMTape3D()};
    auto *tape2d {new TMTape2D()};
    auto *tape1d {new TMTape1D()};
    (*tape2d)[0][0].symbol = "D";
    (*tape2d)[-1][1].symbol = "A";
    const StatePointer startState = std::make_shared<const State>("q0", true, false);
    const StatePointer state2  = std::make_shared<const State>("q1", false, false);
    const StatePointer state3  = std::make_shared<const State>("q2", false, false);

    std::set<StatePointer> states  = {startState, state2, state3};
    FiniteControl control(states, {
            {
                         TransitionDomain(startState, {"B", "D", "B"}),
                    TransitionImage(state2, {"1", "1", "0"}, {Left, Right, Left})
            },
            {
                         TransitionDomain(state2, {"B", "B", "B"}),
                    TransitionImage(state3, {"1", "0", "1"}, {Up, Up, Left}),
            },
            {            TransitionDomain(state3, {"B", "A", "B"}),
                    TransitionImage(state3, {"B", "1", "0"}, {Front, Right, Left}),
            }
    });
    // tuple needs to have pointers of tapes
    std::tuple<TMTape3D*, TMTape2D*, TMTape1D*> tapes = std::make_tuple(tape3d, tape2d, tape1d);
    MTMDTuringMachine<TMTape3D, TMTape2D, TMTape1D> tm({"0", "1"}, {"0", "1"}, tapes, control, printTransition);

    tape2d->print();
    tape1d->print();

    tm.doTransition();
    tape2d->print();
    tape1d->print();

    tm.doTransition();
    tape2d->print();
    tape1d->print();

    tm.doTransition();
    tape2d->print();
    tape1d->print();

    delete tape3d;
    delete tape2d;
    delete tape1d;
    return 0;
}