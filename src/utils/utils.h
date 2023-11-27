//
// Created by daria on 19/11/2023.
//

#ifndef VOXELFUSION_UTILS_H
#define VOXELFUSION_UTILS_H

#include <vector>
#include <string>

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
};


#endif //VOXELFUSION_UTILS_H