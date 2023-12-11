#include <gtest/gtest.h>
#include "LR1Parser/LALR1Parser/LALR1Parser.h"
#include "Lexer/Lexer.h"
#include "string"
#include "TMgenerator/TMGenerator.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "utils/utils.h"

using std::ifstream, std::stringstream, std::make_shared;

class compilationTest : public ::testing::Test {
protected:

    inline static ::shared_ptr<LALR1Parser> parser;
    static void SetUpTestSuite() {
        parser = ::make_shared<LALR1Parser>("src/CFG/input/Tasm.json");
        parser->exportTable("parsingTable.json");
    }
    static shared_ptr<Lexer> initializeLexer(const string& path){
        string code;
        string line;
        ::ifstream input (path);
        if (input.is_open())
        {
            while ( getline (input, line) )
            {
                code += line;
            }
            input.close();
        }
        return make_shared<Lexer>(code);
    };

    static void compareFiles(const string& real, const string& expected){
        ::ifstream t(real);
        ::stringstream buffer;
        buffer << t.rdbuf();
        ::ifstream t2(expected);
        ::stringstream buffer2;
        buffer2 << t2.rdbuf();
        EXPECT_EQ(buffer2.str(), buffer.str());
    }
    static void compile(const string& codePath, shared_ptr<MTMDTuringMachine<TMTape3D, TMTape1D>>& tm){
        auto lexer = initializeLexer(codePath);
        const std::shared_ptr<STNode>& root = parser->parse(lexer->getTokenizedInput());
        root->exportVisualization("test.dot");
        auto *tape3d {new TMTape3D()};
        auto *tape1d {new TMTape1D()};
        auto tapes = std::make_tuple(tape3d, tape1d);
        std::set<std::string> tapeAlphabet = {"B", "S"};
        std::set<StatePointer> states;
        map<TransitionDomain, TransitionImage> transitions;
        TMGenerator generator{tapeAlphabet, transitions, states, false};
        generator.assembleTasm(root);
        FiniteControl control(states, transitions);
        tm = make_shared<MTMDTuringMachine<TMTape3D, TMTape1D>>(tapeAlphabet, tapeAlphabet, tapes, control, nullptr);
    }
    virtual void SetUp() {

    }
    virtual void TearDown() {
    }
};

TEST_F(compilationTest, DiagonalMove)
{
    shared_ptr<MTMDTuringMachine<TMTape3D, TMTape1D>> tm;
    compile("tasm/helloworld.tasm", tm);
    utils::TMtoDotfile(*tm, "tm.dot");
    compareFiles("tm.dot", "src/tests/comparison/diagonalMoveTM.dot");

    tm->doTransitions(21);
    EXPECT_EQ(tm->getFiniteControl().currentState->name, "1");
}
TEST_F(compilationTest, basicConditionals)
{
    shared_ptr<MTMDTuringMachine<TMTape3D, TMTape1D>> tm;
    compile("tasm/conditional.tasm", tm);
    utils::TMtoDotfile(*tm, "tm.dot");
    compareFiles("tm.dot", "src/tests/comparison/basicConditionalsTM.dot");

    tm->doTransitions(14);
    EXPECT_EQ(tm->getFiniteControl().currentState->name, "9");
    EXPECT_EQ(tm->getFiniteControl().currentState->type, State_Accepting);
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}