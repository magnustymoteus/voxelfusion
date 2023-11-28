#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "Visualisation/VisualisationManager.h"
#include <thread>
#include <iostream>
#include "Mesh.h"
#include "utils.h"

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
template<class ...TMTapeType>
void updateVisualisation(const std::tuple<TMTapeType...> &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    VisualisationManager* v = VisualisationManager::getInstance();
    v->setTape(std::get<0>(tapes));

    std::chrono::milliseconds timespan(1000);
    std::this_thread::sleep_for(timespan);
}

int main() {
    auto tape3d {new TMTape3D()};
    (*tape3d)[0][0][0].symbol = "B";
    (*tape3d)[5][0][0].symbol = "D";
    (*tape3d)[0][3][0].symbol = "D";
    (*tape3d)[0][0][2].symbol = "D";
    const StatePointer startState = std::make_shared<const State>("q0", true);

    std::set<StatePointer> states  = {startState};
    FiniteControl control(states, {
            {
                         TransitionDomain(startState, {"D"}),
                    TransitionImage(startState, {"D"}, {Left})
            }
    });

    ////////////////////////
    // Start voxelisation test
    Mesh mesh;
    VoxelSpace voxelSpace;
    auto tape {new TMTape3D()};
    utils::load_obj("tests/parsing/obj/test0.obj", mesh);
    utils::voxelise(mesh, voxelSpace, 0.05);
    utils::voxelSpaceToTape(voxelSpace, *tape, "D");

    // tuple needs to have pointers of tapes
    std::tuple<TMTape3D*> tapes = std::make_tuple(tape);
    MTMDTuringMachine<TMTape3D> tm({"B", "D"}, {"B", "D"}, tapes, control, updateVisualisation);
    VisualisationManager* v = VisualisationManager::getInstance();
    tm.doTransition();
    // End voxelisation test
    ////////////////////////

//    // Start terrain generation test
//    Mesh mesh;
//    VoxelSpace voxelSpace;
//    auto tape {new TMTape3D()};
//    utils::generateTerrain(voxelSpace, 30, 30, 5, 0.5);
//    utils::voxelSpaceToTape(voxelSpace, *tape, "D");
//
//    // tuple needs to have pointers of tapes
//    std::tuple<TMTape3D*> tapes = std::make_tuple(tape);
//    MTMDTuringMachine<TMTape3D> tm({"B", "D"}, {"B", "D"}, tapes, control, updateVisualisation);
//    VisualisationManager* v = VisualisationManager::getInstance();
//    tm.doTransition();
//    // End terrain generation test
//    ////////////////////////

    v->waitForExit();
    delete tape3d;
    return 0;
}