//
// Created by daria on 19/11/2023.
//

#ifndef VOXELFUSION_MESH_H
#define VOXELFUSION_MESH_H

#include <vector>
#include "vector/vector3d.h"
struct Voxel {
    // TODO: add more properties
    bool occupied = false;
};
struct Face {
    Face() {};

    Face(std::vector<int> inds):point_indexes(inds) {};

    std::vector<int> point_indexes;
};
struct Mesh {
    Mesh() = default;
    std::vector<Vector3D> points;
    std::vector<Face> faces;
};

struct BoundingBox {
    double minX, minY, minZ;
    double maxX, maxY, maxZ;
};

#endif //VOXELFUSION_MESH_H
