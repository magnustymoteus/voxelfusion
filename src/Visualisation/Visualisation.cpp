#include "Visualisation.h"
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

void createCube(vector<GLfloat>& vertices, vector<GLuint>& indices, int x, int y, int z, float scale) {
    int startingIndex = vertices.size()/3;
    for(unsigned int i = 0; i < baseVertices.size(); i += 3)
    {
        vertices.push_back(baseVertices[i] * scale + x);
        vertices.push_back(baseVertices[i+1] * scale + y);
        vertices.push_back(baseVertices[i+2] * scale + z);
    }
    for (unsigned int i = 0; i < baseIndices.size(); i++)
    {
        indices.push_back(startingIndex + baseIndices[i]);
    }
}

Visualisation::Visualisation() {
    glfwInit();

    // general opengl settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "yes", NULL, NULL);
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
    shaderProgram = new Shader("vertex.glsl", "fragment.glsl");
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
    camera->Matrix(45.0f, 0.1f, 100.0f, *shaderProgram, "cameraMatrix");

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

    const long greatest2DSize = static_cast<long>(std::max_element(tape->cells.begin(), tape->cells.end(),
                                                                 [](const TMTape2D &a, const TMTape2D &b) {
                                                                     return a.cells.size() < b.cells.size();
                                                                 })->cells.size());
    for (long x = 0; x < tape->cells.size(); x++) {
        TMTape2D& currentCellRow = tape->cells[x];
        const long greatestSize = static_cast<long>(std::max_element(currentCellRow.cells.begin(), currentCellRow.cells.end(),
                                                                     [](const TMTape1D &a, const TMTape1D &b) {
                                                                         return a.cells.size() < b.cells.size();
                                                                     })->cells.size());
        for(long y= -greatest2DSize / 2; y <= greatest2DSize / 2; y++) {
            for(long z= -greatestSize / 2; z <= greatestSize / 2; z++) {
                if(currentCellRow[y][z].symbol != "B"){
                    createCube(vertices, indices, x, y, z, 1);
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
        VAO->LinkVertexBuffer(*VBO, 0);

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