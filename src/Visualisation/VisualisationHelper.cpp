#include <fstream>
#include "VisualisationHelper.h"

vector<GLfloat> baseVertices = {
        0, 0, 0,    0, -1, 0, //face 0
        1, 0, 0,    0, -1, 0,
        1, 0, 1,    0, -1, 0,
        0, 0, 1,    0, -1, 0,
        0, 1, 0,    -1, 0, 0, //face 1
        0, 0, 0,    -1, 0, 0,
        0, 0, 1,    -1, 0, 0,
        0, 1, 1,    -1, 0, 0,
        0, 1, 1,    0, 0, 1, //face 2
        0, 0, 1,    0, 0, 1,
        1, 0, 1,    0, 0, 1,
        1, 1, 1,    0, 0, 1,
        1, 0, 0,    1, 0, 0, //face 3
        1, 1, 0,    1, 0, 0,
        1, 1, 1,    1, 0, 0,
        1, 0, 1,    1, 0, 0,
        1, 1, 0,    0, 1, 0, //face 4
        0, 1, 0,    0, 1, 0,
        0, 1, 1,    0, 1, 0,
        1, 1, 1,    0, 1, 0,
        1, 0, 0,    0, 0, -1, //face 5
        0, 0, 0,    0, 0, -1,
        0, 1, 0,    0, 0, -1,
        1, 1, 0,    0, 0, -1
};

vector<GLuint> baseIndices = {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        4, 6, 7,
        8, 9, 10,
        8, 10, 11,
        12, 13, 14,
        12, 14, 15,
        16, 17, 18,
        16, 18, 19,
        20, 21, 22,
        20, 22, 23
};

void VisualisationHelper::createCube(vector<GLfloat> &vertices, vector<GLuint> &indices, int x, int y, int z, float scale, const Color &color) {
    int startingIndex = vertices.size()/10;
    for(unsigned int i = 0; i < baseVertices.size(); i += 6)
    {
        vertices.push_back(baseVertices[i] * scale + x);
        vertices.push_back(baseVertices[i+1] * scale + y);
        vertices.push_back(baseVertices[i+2] * scale + z);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        vertices.push_back(color.a);
        vertices.push_back(baseVertices[i+3]); // normals
        vertices.push_back(baseVertices[i+4]);
        vertices.push_back(baseVertices[i+5]);
    }
    for (unsigned int i = 0; i < baseIndices.size(); i++)
    {
        indices.push_back(startingIndex + baseIndices[i]);
    }
}
void VisualisationHelper::exportMesh(vector<GLfloat> &vertices, vector<GLuint> &indices, const string &filename) {
    ofstream objFile;
    objFile.open (filename);

//    for(auto& color: colorMap){
//        objFile << "newmtl " << color.first << '\n';
//        objFile << "Ka " << color.second.r << " " << color.second.g << " " << color.second.b << '\n';
//    }
    for (int i = 0; i < vertices.size(); i+=10) {
        objFile << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << "\n";
    }
    int elementsPerCube = baseIndices.size() * 3;
    for (int i = 0; i < indices.size(); ) {
        //objFile << "usemtl " << *faceColors[i / elementsPerCube] << '\n';
        for (int j = 0; j < baseIndices.size(); ++j) {
            objFile << "f " << indices[i] + 1 << " " << indices[i + 1] + 1 << " " << indices[i + 2] + 1 << "\n";
            i+= 3;
        }
    }
    objFile.close();

}