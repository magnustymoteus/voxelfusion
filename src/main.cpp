#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "Visualisation/VisualisationManager.h"
#include <thread>
#include <iostream>
#include <algorithm>
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
void updateVisualisation(const std::tuple<TMTapeType*...> & tapes, const std::vector<unsigned int> changedTapesIndices) {
    if (std::find(changedTapesIndices.begin(), changedTapesIndices.end(), 0) == changedTapesIndices.end()) return;
    VisualisationManager* v = VisualisationManager::getInstance();
    v->setTape(std::get<0>(tapes));
//
//    std::chrono::milliseconds timespan(1000);
//    std::this_thread::sleep_for(timespan);
}
#include "LR1Parser/LALR1Parser/LALR1Parser.h"
#include "Lexer/Lexer.h"
#include <fstream>
#include "string"
#include "TMgenerator/TMGenerator.h"
using namespace std;
int main() {
    // Step 1: read tasm code
    string code;
    string line;
    ifstream input ("tasm/water-physics.tasm");
    if (input.is_open())
    {
        while ( getline (input, line) )
        {
            code += line;
        }
        input.close();
    }
    // Step 2: get the lexicon of the code
    Lexer lexer(code);
    lexer.print();
    // Step 3: parse the code
    // Step 3.2: create parse table
    LALR1Parser parserTemp("src/CFG/input/Tasm.json");
    parserTemp.exportTable("parsingTable.json");
    // Step 3.3: import parse table
    LALR1Parser parser;
    parser.importTable("parsingTable.json");
    // Step 3.4: parse the table
    const std::shared_ptr<STNode>& root = parser.parse(lexer.getTokenizedInput());
    // Step 3.5 (optional) export dot visualization
    root->exportVisualization("test.dot");
    // Step 4: Create and assemble all tapes
    auto *tape3d {new TMTape3D()};
    auto *varTape {new TMTape1D()};
    auto *tempVarTape {new TMTape1D()};
    auto *historyTape {new TMTape3D()};
    auto tapes = std::make_tuple(tape3d, varTape, tempVarTape, historyTape);
    std::set<std::string> tapeAlphabet = {"B", "GW", "W", "G"};
    std::set<StatePointer> states;
    map<TransitionDomain, TransitionImage> transitions;
    // Step 4.2: put tasm on the tapes
    TMGenerator generator{tapeAlphabet, transitions, states, true};
    generator.assembleTasm(root);
    FiniteControl control(states, transitions);
    MTMDTuringMachine<TMTape3D, TMTape1D, TMTape1D, TMTape3D> tm(tapeAlphabet, tapeAlphabet, tapes, control, updateVisualisation);
    //utils::TMtoDotfile(tm, "tm.dot");
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

    tm.doTransitions();
    v->waitForExit();
    delete tape3d;
    delete varTape;
    delete tempVarTape;
    delete historyTape;
//    delete tape1d;
    return 0;
}