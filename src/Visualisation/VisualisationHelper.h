#ifndef VOXELFUSION_VISUALISATIONHELPER_H
#define VOXELFUSION_VISUALISATIONHELPER_H

#include <vector>
#include <GLFW/glfw3.h>
#include <string>

using namespace std;

struct Color{
    float r;
    float g;
    float b;
    float a;

    Color(float r, float g, float b, float a);
};

struct VisualisationHelper {
    static void createCube(vector<GLfloat> &vertices, vector<GLuint> &indices, int x, int y, int z, float scale, const Color &color);
    static void exportMesh(vector<GLfloat> &vertices, vector<GLuint> &indices, const string &filename);
};


#endif //VOXELFUSION_VISUALISATIONHELPER_H
