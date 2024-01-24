#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <thread>
#include <memory>
#include <map>
#include "imgui.h"


// Class structure roughly based upon https://www.youtube.com/watch?v=greXpRqCTKs&list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-&index=5
#include "Visualisation/Shader.h"
#include "Visualisation/VertexArray.h"
#include "Visualisation/VertexBuffer.h"
#include "Visualisation/ElementBuffer.h"
#include "Visualisation/Camera.h"
#include "Visualisation/VisualisationHelper.h"
#include "Visualisation/AtomicQueue.h"

#include "MTMDTuringMachine/TMTape.h"
#include "TMgenerator/TMGenerator.h"
#include "MTMDTuringMachine/MTMDTuringMachine.h"
#include "LR1Parser/LALR1Parser/LALR1Parser.h"
using namespace std;


class Visualisation {
    GLFWwindow* window;
    Shader* shaderProgram;
    Camera* camera;
    VertexArray* VAO{nullptr};
    VertexBuffer* VBO{nullptr};
    ElementBuffer* EBO{nullptr};
    vector<GLfloat> vertices;
    vector<GLuint> indices;
    vector<const string*> faceColors;
    float FOV;
    float nearPlane;
    float farPlane;
    const map<string, Color>& colorMap;
    float sunLocation[3]{ 1,1,1 };
    ImVec4 sunColor{1.0f, 1.0f, 1.0f, 1.0f};
    ImVec4 backgroundColor{0.07f, 0.13f, 0.17f, 1.0f};
    unique_ptr<TMTape3D> tape;
    unique_ptr<thread> TMworker;
    unique_ptr<thread> objLoader;
    const string tasmBasePath = "tasm/";
    const string objBasePath = "objs/";
    vector<string> tasmPaths;
    vector<string> objPaths;
    vector<bool> tasmPathsSelected;
    vector<bool> objPathsSelected;
    string selectedTasmPath;
    string selectedObjPath;

    inline static std::atomic<bool> tmRunning = false;
    inline static std::atomic<bool> objLoaderRunning = false;
public:
    inline static std::atomic<bool> updateFlag = false;
    Visualisation(float fov, float nearPlane, float farPlane, map<string, Color>& colorMap);
    bool update();
    void rebuild(TMTape3D *tape);
    void run();
    ~Visualisation();
    void imguiBeginFrame() const;
    void imguiDrawAndHandleFrame();
    void killAndWaitForTMworker();
    void killAndWaitForOBJloader();

    void runTM();

    void resetTape();
};
