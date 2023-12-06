#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "Visualisation/VisualisationManager.h"
#include <thread>
#include <iostream>
#include "utils/utils.h"

using namespace std;
/*template<class ...TMTapeType>
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
}*/
template<class ...TMTapeType>
void updateVisualisation(const std::tuple<TMTapeType...> &tapes, const TransitionDomain &domain, const TransitionImage &image) {
    VisualisationManager* v = VisualisationManager::getInstance();
    v->setTape(std::get<0>(tapes));

    std::chrono::milliseconds timespan(1000);
    std::this_thread::sleep_for(timespan);
}
#include "LR1Parser/LALR1Parser/LALR1Parser.h"
#include "Lexer/Lexer.h"
#include <fstream>
#include "string"
#include "TMgenerator/TMGenerator.h"
using namespace std;
int main() {
    string code;
    string line;
    ifstream input ("tasm/helloworld.tasm");
    if (input.is_open())
    {
        while ( getline (input, line) )
        {
            code += line;
        }
        input.close();
    }

    Lexer lexer(code);
    lexer.print();
    LALR1Parser parser("src/CFG/input/Tasm.json");
    parser.exportTable("parsingTable.json");
    const std::shared_ptr<STNode>& root = parser.parse(lexer.getTokenizedInput());
    root->exportVisualization("test.dot");
    auto *tape3d {new TMTape3D()};
    std::tuple tapes = std::make_tuple(tape3d);
    //MTMDTuringMachine<TMTape3D, TMTape2D, TMTape1D> tm({}, {}, tapes, control, updateVisualisation);
    shared_ptr<MTMDTuringMachine<TMTape3D>> tm;
    std::set<std::string> tapeAlphabet = {"B", "S"};
    TMGenerator generator{tm, tapeAlphabet, tapeAlphabet, tapes, updateVisualisation};
    generator.assembleTasm(root);
    utils::TMtoDotfile(*tm, "tm.dot");
    /*auto *tape3d {new TMTape3D()};
    auto *tape2d {new TMTape2D()};
    auto *tape1d {new TMTape1D()};
    (*tape2d)[0][0].symbol = "D";
    (*tape2d)[-1][1].symbol = "A";
    const StatePointer startState = std::make_shared<const State>("q0", true);
    const StatePointer state2  = std::make_shared<const State>("q1", false);
    const StatePointer state3  = std::make_shared<const State>("q2", false);
    const StatePointer state4  = std::make_shared<const State>("q3", false);

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
                TransitionImage(state4, {"1", "1", "0"}, {Front, Right, Left}),
        },
        {            TransitionDomain(state4, {"B", "B", "B"}),
                TransitionImage(state4, {"1", "B", "B"}, {Front, Right, Left}),
        }
        });
    // tuple needs to have pointers of tapes
    std::tuple<TMTape3D*, TMTape2D*, TMTape1D*> tapes = std::make_tuple(tape3d, tape2d, tape1d);
    MTMDTuringMachine<TMTape3D, TMTape2D, TMTape1D> tm({"0", "1"}, {"0", "1"}, tapes, control, updateVisualisation);*/
    VisualisationManager* v = VisualisationManager::getInstance();

    tm->doTransitions(20);
    v->waitForExit();
    delete tape3d;
//    delete tape2d;
//    delete tape1d;
    return 0;
}