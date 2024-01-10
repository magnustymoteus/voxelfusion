#include <algorithm>
#include "Visualisation.h"
#include "../MTMDTuringMachine/TMTapeUtils.h"
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 800

vector<GLfloat> baseVertices = {
        1, 0, 0,
        0, 1, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 0,
        0, 0, 0,
        1, 0, 1,
        0, 1, 1,
};

vector<GLuint> baseIndices = {
        0, 4, 2,
        0, 2, 6,
        4, 1, 7,
        4, 7, 2,
        1, 5, 3,
        1, 3, 7,
        5, 0, 6,
        5, 6, 3,
        6, 2, 7,
        6, 7, 3,
        0, 5, 1,
        0, 1, 4
};

void
createCube(vector<GLfloat> &vertices, vector<GLuint> &indices, int x, int y, int z, float scale, const Color &color) {
    int startingIndex = vertices.size()/7;
    for(unsigned int i = 0; i < baseVertices.size(); i += 3)
    {
        vertices.push_back(baseVertices[i] * scale + x);
        vertices.push_back(baseVertices[i+1] * scale + y);
        vertices.push_back(baseVertices[i+2] * scale + z);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        vertices.push_back(color.a);
    }
    for (unsigned int i = 0; i < baseIndices.size(); i++)
    {
        indices.push_back(startingIndex + baseIndices[i]);
    }
}

Visualisation::Visualisation(float fov, float nearPlane, float farPlane, map<string, Color>& colorMap) :
FOV(fov), nearPlane(nearPlane), farPlane(farPlane), colorMap(colorMap) {
    glfwInit();

    // general opengl settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Say 'Oh my god, it's so beautiful!'", NULL, NULL);
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
    TMTape3D tape;
    rebuild(&tape);

    glEnable(GL_DEPTH_TEST);

    camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f));

    shaderProgram->Activate();
}
bool Visualisation::update() {
    if (glfwWindowShouldClose(window)) return false;

    glfwPollEvents();
    //prepare command to draw on back buffer
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    //execute said command
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera->Inputs(window);
    camera->Matrix(FOV, nearPlane, farPlane, *shaderProgram, "cameraMatrix");

    VAO->Bind();

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    //show the buffer
    glfwSwapBuffers(window);
    return true;
}

void Visualisation::rebuild(TMTape3D *tape) {
    //https://stackoverflow.com/questions/15821969/what-is-the-proper-way-to-modify-opengl-vertex-buffer
    vertices.clear();
    indices.clear();

    const int greatest3DSize = tape->getCells().size();
    const int greatest2DSize = TMTapeUtils::getGreatestSize(tape->getCells());
    const long greatestSize = TMTapeUtils::getGreatestSize(tape->at(0).cells);
    for (int x= -greatest3DSize / 2; x <= greatest3DSize / 2; x++) {
        for(int y= -greatest2DSize / 2; y <= greatest2DSize / 2; y++) {
            for(int z= -greatestSize / 2; z <= greatestSize / 2; z++) {
                string symbol = tape->at(x).at(y).at(z).symbol;
                if(symbol != "B"){
                    if(colorMap.find(symbol) == colorMap.end()){
                        createCube(vertices, indices, x, y, z, 1, colorMap.at("default"));
                    }else{
                        createCube(vertices, indices, x, y, z, 1,  colorMap.at(symbol));
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
        VAO->LinkVertexBufferAttribute(*VBO, 0, 3, GL_FLOAT, 7 * sizeof(GLfloat), (void *) 0);
        VAO->LinkVertexBufferAttribute(*VBO, 1, 4, GL_FLOAT, 7 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));

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
    VAO->Delete();
    VBO->Delete();
    EBO->Delete();
    shaderProgram->Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
}

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
