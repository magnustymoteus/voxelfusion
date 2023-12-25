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
    static void finiteControlToDotfile(FiniteControl& control, const std::string& path);
public:
    static void load_obj(const std::string& path, Mesh& mesh);
    static BoundingBox calculateBoundingBox(const Mesh& mesh);
    static void translateAndScale(Vector3D& point, const Vector3D& translatePoint, double scaler);
    static bool voxelTriangleIntersection(const int& x, const int& y, const int& z, const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
    static void voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize=1);
    static void voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol="X", bool edge=false); // TODO: fillSymbol more flexible?
    static void objToTape(const std::string& path, TMTape3D& tape, const double& voxelSize=1, const std::string& fillSymbol="X", bool edge=false);
    static void generateTerrain(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, const double& scale);
    template<class... TMTapeType>
    static void TMtoDotfile(MTMDTuringMachine<TMTapeType...> TM, const std::string& path) {
        FiniteControl control = TM.getFiniteControl();
        finiteControlToDotfile(control, path);
    }
};


#endif //VOXELFUSION_UTILS_H
