//

#include "OBJParser.h"
#include <fstream>
#include <sstream>

Mesh OBJParser::parse(const std::string objPath) const {
    std::ifstream file(objPath);
    if(!file.is_open()) {
        throw std::runtime_error("Cannot open .obj file "+objPath);
    }

    Mesh result;
    std::string currentLine;
    while(std::getline(file, currentLine)) {
        std::istringstream sstream(currentLine);
        std::string currentToken;
        sstream >> currentToken;
        if(currentToken == "v") {
            Vector3D point;
            sstream >> point.x >> point.y >> point.z;
            result.points.push_back(point);
        }
        else if(currentToken == "f") {
            Face face;
            int pointIndex;
            while(sstream >> pointIndex) face.point_indexes.push_back(pointIndex);
            result.faces.push_back(face);
        }
    }

    file.close();
    return result;
}