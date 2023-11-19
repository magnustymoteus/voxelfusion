//#include "CFG/AugmentedCFG.h"
//#include "CFG/CFGUtils.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "Visualisation/VisualisationManager.h"
#include <thread>
#include <iostream>
using namespace std;
void printTransition(const TMTapes &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    std::cout << domain.state.name << " -> " << image.state.name << std::endl;
    for(const std::string &currentDomainSymbol : domain.replacedSymbols) {
        std::cout << currentDomainSymbol << " ";
    }
    std::cout << "replaced by ";
    for(const std::string &currentImageSymbol : image.replacementSymbols) {
        std::cout << currentImageSymbol << " ";
    }
    std::cout << std::endl;
    for(const TMTapeDirection &currentDirection : image.directions) {
        std::cout << "Move tapehead to direction " << static_cast<char>(currentDirection) << std::endl;
    }
    std::cout << std::endl;
}

void updateVisualisation(const TMTapes &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    VisualisationManager* v = VisualisationManager::getInstance();
    v->setTape(dynamic_cast<TMTape3D*>(tapes.at(0)));

    std::chrono::milliseconds timespan(1000);
    std::this_thread::sleep_for(timespan);
}

int main() {
    /*AugmentedCFG aCfg("CFG/input/CFG.json");
    ItemSet nextItemSet;
    nextItemSet.insert({"T", aCfg.getItemSet().at("T")});
    CFGUtils::print(aCfg.getItemSet());
    std::cout << std::endl;
    CFGUtils::print(aCfg.computeClosure(nextItemSet));*/

    // example
    auto *tape2d = new TMTape2D();
    (*tape2d)[0][0].symbol = "D";
    (*tape2d)[-1][1].symbol = "A";
    tape2d->print();
    delete tape2d;

    auto tape3d = new TMTape3D();
    (*tape3d)[0][0][0].symbol = "D";
    (*tape3d)[0][0][1].symbol = "A";
    const StatePointer startState = std::make_shared<const State>("q0", true, false);
    const StatePointer state2  = std::make_shared<const State>("q1", false, false);
    const StatePointer state3  = std::make_shared<const State>("q2", false, false);

    std::set<StatePointer> states  = {startState, state2, state3};
    FiniteControl control(states, {
        {
            TransitionDomain(startState, {"D"}),
            TransitionImage(state2, {"1"}, {Right})
        },
        {
            TransitionDomain(state2, {"A"}),
            TransitionImage(state3, {"0"}, {Right}),
        },
        {            TransitionDomain(state3, {"B"}),
                TransitionImage(state3, {"1"}, {Right}),
        }
        });
    VisualisationManager* v = VisualisationManager::getInstance();

    TMTapes tapes = {tape3d};
    MTMDTuringMachine tm({"0", "1"}, {"0", "1"}, tapes, control, updateVisualisation);

    for (int i = 0; i < 5; ++i) {
        tm.doTransition();
    }

    delete tape3d;
    v->waitForExit();
    return 0;
}