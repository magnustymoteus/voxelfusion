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


// Class structure roughly based upon https://www.youtube.com/watch?v=greXpRqCTKs&list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-&index=5
#include "Visualisation/Shader.h"
#include "Visualisation/VertexArray.h"
#include "Visualisation/VertexBuffer.h"
#include "Visualisation/ElementBuffer.h"
#include "Visualisation/Camera.h"

#include "MTMDTuringMachine/TMTape.h"

class Visualisation {
    GLFWwindow* window;
    Shader* shaderProgram;
    Camera* camera;
    VertexArray* VAO{nullptr};
    VertexBuffer* VBO{nullptr};
    ElementBuffer* EBO{nullptr};
    vector<GLfloat> vertices;
    vector<GLuint> indices;
public:
    Visualisation();
    bool update();
    void rebuild(TMTape3D &tape);
    ~Visualisation();
};
