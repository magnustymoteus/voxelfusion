#include "Visualisation.h"
#include "../MTMDTuringMachine/TMTapeUtils.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <thread>
#include <iostream>
#include "LR1Parser/LALR1Parser/LALR1Parser.h"
#include "Lexer/Lexer.h"
#include "string"
#include "TMgenerator/TMGenerator.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "utils.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 800

void updateVisualisation(const std::tuple<TMTape3D*, TMTape1D*, TMTape1D*, TMTape3D*> & tapes, const std::vector<unsigned int> changedTapesIndices) {
    if (std::find(changedTapesIndices.begin(), changedTapesIndices.end(), 0) == changedTapesIndices.end()) return;
    Visualisation::updateFlag = true;
}

Visualisation::Visualisation(float fov, float nearPlane, float farPlane, map<string, Color>& colorMap) :
FOV(fov), nearPlane(nearPlane), farPlane(farPlane), colorMap(colorMap) {
    glfwInit();

    // general opengl settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "VoxelFusion", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // let glad configure OpenGL
    gladLoadGL();

    //what part of the window can OpenGL draw on
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Compile shader from files
    shaderProgram = new Shader("src/Visualisation/vertex.glsl", "src/Visualisation/fragment.glsl");

    glEnable(GL_DEPTH_TEST);

    camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f));
    shaderProgram->Activate();

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    rebuild(nullptr);


    for (const auto & entry : std::filesystem::directory_iterator(tasmBasePath)){
        tasmPaths.push_back(entry.path().string());
        tasmPathsSelected.push_back(false);
    }
    tasmPathsSelected.reserve(tasmPaths.size());
    for (const auto & entry : std::filesystem::directory_iterator(objBasePath)){
        objPaths.push_back(entry.path().string());
        objPathsSelected.push_back(false);
    }
}

void Visualisation::runTM() {
    assert(tmRunning == false && tape != nullptr);
    tmRunning = true;
    cachedTMRunning = true;
    // Step 1: read tasm code
    string code;
    string line;
    ifstream input (selectedTasmPath);
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
// Step 3.3: import parse table
    LALR1Parser parser;
    parser.importTable("parsingTable.json");
    // Step 3.4: parse the table
    const shared_ptr<STNode>& root = parser.parse(lexer.getTokenizedInput());
    // Step 4: Create and assemble all tapes
    auto *varTape {new TMTape1D()};
    auto *tempVarTape {new TMTape1D()};
    auto *historyTape {new TMTape3D()};
    auto tapes = make_tuple(tape.get(), varTape, tempVarTape, historyTape);
    set<string> tapeAlphabet = {"B"};
    set<StatePointer> states;
    map<TransitionDomain, TransitionImage> transitions;
    // Step 4.2: put tasm on the tapes
    TMGenerator generator{tapeAlphabet, transitions, states, true};
    generator.assembleTasm(root);
    FiniteControl control(states, transitions);
    MTMDTuringMachine<TMTape3D, TMTape1D, TMTape1D, TMTape3D> tm(tapeAlphabet, tapeAlphabet, tapes, control, updateVisualisation);

    transitionsMadeLast = 0;
    int currentTransitionsMade = 0;
    while(!tm.isHalted){
        tm.doTransition();
        currentTransitionsMade++;
#ifdef ALLOW_TM_PREEMPTION // this slows down very large TASM scripts because of mutex locking and unlocking
        if(currentTransitionsMade % 50 == 0){
            if(!tmRunning) break;
        }
#endif
    }
    transitionsMadeLast = currentTransitionsMade;
    delete varTape;
    delete tempVarTape;
    delete historyTape;
    tmRunning = false;
    cachedTMRunning = false;
}

bool Visualisation::update() {
    if (glfwWindowShouldClose(window)) return false;
    glfwPollEvents();
    //prepare command to draw on back buffer
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
    //execute said command
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (updateFlag){
        updateFlag = false;
        if (tape == nullptr) {
            cout << "Tape is null" << endl;
        }
        rebuild(tape.get());
    }

    imguiBeginFrame();


    glUniform3f(glGetUniformLocation(shaderProgram->ID, "sunPosition"), sunLocation[0], sunLocation[1], sunLocation[2]);
    glUniform4f(glGetUniformLocation(shaderProgram->ID, "sunColor"), sunColor.x, sunColor.y, sunColor.z, sunColor.w);
    if(!ImGui::GetIO().WantCaptureMouse){
        camera->Inputs(window);
    }
    camera->Matrix(FOV, nearPlane, farPlane, *shaderProgram, "cameraMatrix");

    VAO->Bind();

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    imguiDrawAndHandleFrame();

    //show the buffer
    glfwSwapBuffers(window);
    return true;
}

void Visualisation::imguiDrawAndHandleFrame() {
    ImGui::Begin("VoxelFusion Control Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    if(objLoaderRunning){
        ImGui::Text("Loading OBJ...");
    }

    if(cachedTMRunning){
        ImGui::Text("Running TASM...");
    }
    if(transitionsMadeLast > 0){
        ImGui::Text("Transitions made in last TM run: %d", transitionsMadeLast);
    }
    if (ImGui::TreeNode("Import model"))
    {
        ImGui::BeginChild("Pick a model");
        for (int i = 0; i < objPaths.size(); i++){
            if (ImGui::Selectable(objPaths[i].c_str(), objPathsSelected[i], ImGuiSelectableFlags_AllowDoubleClick))
                if (ImGui::IsMouseDoubleClicked(0)) {
                    selectedObjPath = objPaths[i];
                    killAndWaitForTMworker();
                    killAndWaitForOBJloader();
                    if(tape == nullptr) tape = make_unique<TMTape3D>();
                    objLoader = make_unique<thread>([this]{
                        assert(objLoaderRunning == false && tape != nullptr && !selectedObjPath.empty());
                        objLoaderRunning = true;
                        utils::objToTape(selectedObjPath, *tape, 0.1, "A");
                        updateFlag = true;
                        objLoaderRunning = false;
                    });
                    cout << "Picked a model to import" << endl;
                }
        }
        ImGui::EndChild();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Run TASM"))
    {
        ImGui::BeginChild("Pick a script");
        for (int i = 0; i < tasmPaths.size(); i++){
            if (ImGui::Selectable(tasmPaths[i].c_str(), tasmPathsSelected[i], ImGuiSelectableFlags_AllowDoubleClick))
                if (ImGui::IsMouseDoubleClicked(0)){
                    selectedTasmPath = tasmPaths[i];
                    if (tape == nullptr) tape = make_unique<TMTape3D>();
                    killAndWaitForOBJloader();
                    killAndWaitForTMworker();
                    TMworker = make_unique<thread>([this]{
                        runTM();
                    });
                    cout << "Picked a script to run" << endl;
                }
        }
        ImGui::EndChild();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Tape"))
    {
        if (ImGui::Button("Reset tape")){
            cout << "Reset Tape Button pressed" << endl;
            resetTape();
            rebuild(nullptr);
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Light & background"))
    {
        ImGui::DragFloat3("Sun location", sunLocation, 1);
        ImGui::ColorEdit3("Sun light color", (float*)&sunColor);
        ImGui::ColorEdit3("Background color", (float*)&backgroundColor);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Export to OBJ"))
    {
        if (ImGui::Button("Do it")){
            cout << "Export Button pressed" << endl;
            VisualisationHelper::exportMesh(vertices, indices, "export.obj");
        }
        ImGui::TreePop();
    }

    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Visualisation::resetTape() {
    killAndWaitForTMworker();
    killAndWaitForOBJloader();
    tape = nullptr;
}

void Visualisation::imguiBeginFrame() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Visualisation::rebuild(TMTape3D *tape) {
    //https://stackoverflow.com/questions/15821969/what-is-the-proper-way-to-modify-opengl-vertex-buffer
    vertices.clear();
    indices.clear();

    if(tape){
        TMTapeUtils::expansionMutex.lock();
        const int greatest3DSize = tape->getCells().size();
        const int greatest2DSize = TMTapeUtils::getGreatestSize(tape->getCells());
        const long greatestSize = TMTapeUtils::getGreatestSize(tape->at(0).cells);
        TMTapeUtils::expansionMutex.unlock();
        for (int x= -tape->zeroAnchor; x < greatest3DSize; x++) {
            for(int y= -tape->at(x).zeroAnchor; y < greatest2DSize; y++) {
                for(int z= -tape->at(x).at(y).zeroAnchor; z < greatestSize; z++) {
                    TMTapeUtils::expansionMutex.lock();
                    string symbol = tape->at(x).at(y).at(z).symbol;
                    TMTapeUtils::expansionMutex.unlock();
                    if(symbol != "B" && symbol != "BB"){
                        auto it = colorMap.find(symbol);
                        if(it == colorMap.end()){
                            it = colorMap.find("default");
                        }
                        VisualisationHelper::createCube(vertices, indices, x, y, z, 1, it->second);
                    }
                }
            }
        }
    }

    if(!VAO){ //first time
        VAO = new VertexArray();
        VAO->Bind();

        VBO = new VertexBuffer(&vertices[0], vertices.size()*sizeof(GLfloat));
        EBO = new ElementBuffer(&indices[0], indices.size() * sizeof(GLfloat));

        EBO->Bind();
        VAO->LinkVertexBufferAttribute(*VBO, 0, 3, GL_FLOAT, 10 * sizeof(GLfloat), (void *) 0);
        VAO->LinkVertexBufferAttribute(*VBO, 1, 4, GL_FLOAT, 10 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
        VAO->LinkVertexBufferAttribute(*VBO, 2, 3, GL_FLOAT, 10 * sizeof(GLfloat), (void *) (7 * sizeof(GLfloat)));

    }else{
        VAO->Bind();
        VBO->redefine(&vertices[0], vertices.size()*sizeof(GLfloat));
        EBO->redefine(&indices[0], indices.size()*sizeof(GLfloat));
        EBO->Bind();
    }
    VAO->Unbind();
    VBO->Unbind();
    EBO->Unbind();
}

Visualisation::~Visualisation() {

    killAndWaitForTMworker();
    killAndWaitForOBJloader();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete camera;
    VAO->Delete();
    VBO->Delete();
    EBO->Delete();
    shaderProgram->Delete();
    delete shaderProgram;
    delete VAO;
    delete VBO;
    delete EBO;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Visualisation::run() {
    while(update());
}

void Visualisation::killAndWaitForTMworker() {
    tmRunning = false;
    if(TMworker) TMworker->join();
    TMworker = nullptr;
}

void Visualisation::killAndWaitForOBJloader() {
    objLoaderRunning = false;
    if (objLoader) objLoader->join();
    objLoader = nullptr;
}

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
