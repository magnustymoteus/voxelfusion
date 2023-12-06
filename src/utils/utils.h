//
// Created by daria on 19/11/2023.
//

#ifndef VOXELFUSION_UTILS_H
#define VOXELFUSION_UTILS_H

#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include "../MTMDTuringMachine/MTMDTuringMachine.h"

struct Voxel;
class TMTape3D;
struct Mesh;
struct BoundingBox;
class Vector3D;

using VoxelSpace = std::vector<std::vector<std::vector<Voxel>>>;

class utils {
public:
    static void load_obj(const std::string& path, Mesh& mesh);
    static BoundingBox calculateBoundingBox(const Mesh& mesh);
    static void translateAndScale(Vector3D& point, const Vector3D& translatePoint, double scaler);
    static bool voxelTriangleIntersection(const int& x, const int& y, const int& z, const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
    static void voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize=1);
    static void voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol="X", bool edge=false); // TODO: fillSymbol more flexible?
    static void generateTerrain(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, const double& scale);
    template<class... TMTapeType>
    static void TMtoDotfile(MTMDTuringMachine<TMTapeType...> TM, const std::string& path) {
        std::string result = "DiGraph G {\n";
        auto control = TM.getFiniteControl();
        for(auto transition: control.transitions){
            result += transition.first.state.name;
            result += "->";
            std::string replaced = std::accumulate(transition.first.replacedSymbols.begin(), transition.first.replacedSymbols.end(), std::string(""));
            std::string replacedBy = std::accumulate(transition.second.replacementSymbols.begin(), transition.second.replacementSymbols.end(), std::string(""));
            std::string direction;
            for(auto& dir: transition.second.directions){
                char a = static_cast<char>(dir);
                direction.push_back(a);
                direction += " | ";
            }
            result += transition.second.state.name + "[label=\"" + replaced + '\\' + "/" + replacedBy + ", " + direction + "\"]" + '\n';
        }
        result += "}";
        std::ofstream output(path);
        output << result;
        output.close();
    }
};


#endif //VOXELFUSION_UTILS_H
