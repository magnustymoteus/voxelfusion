#pragma once
#include <iostream>
using namespace std;
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


// Class structure roughly based upon https://www.youtube.com/watch?v=greXpRqCTKs&list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-&index=5
#include "Visualisation/Shader.h"
#include "Visualisation/VertexArray.h"
#include "Visualisation/VertexBuffer.h"
#include "Visualisation/ElementBuffer.h"
#include "Visualisation/Camera.h"

#include "MTMDTuringMachine/TMTape.h"


struct Color{
    float r;
    float g;
    float b;
    float a;

    Color(float r, float g, float b, float a);
};

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
public:
    Visualisation(float fov, float nearPlane, float farPlane, map<string, Color>& colorMap);
    bool update();
    void rebuild(TMTape3D *tape);
    void exportMesh(const string& filename);
    ~Visualisation();
};
