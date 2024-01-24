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
#include "Mesh.h"

struct Voxel;
class TMTape3D;
struct Mesh;
struct BoundingBox;
class Vector3D;

using VoxelSpace = std::vector<std::vector<std::vector<Voxel>>>;

class utils {
    static void finiteControlToDotfile(FiniteControl& control, const std::string& path);
public:
    /**
     * This function converts obj file to mesh with obj_parser.h
     * @param path path to the obj file
     * @param mesh the output mesh
     */
    static void load_obj(const std::string& path, Mesh& mesh);
    /**
     * This function converts obj file to mesh with our own parser
     * @param path path to the obj file
     * @param mesh the output mesh
     */
    static void load_obj2(const std::string& path, Mesh& mesh);
    static BoundingBox calculateBoundingBox(const Mesh& mesh);
    static void translateAndScale(Vector3D& point, const Vector3D& translatePoint, double scaler);
    static bool voxelTriangleIntersection(const int& x, const int& y, const int& z, const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
    static void voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize=1);
    static void voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol="X", bool edge=false);
    static void objToTape(const std::string& path, TMTape3D& tape, const double& voxelSize=1, const std::string& fillSymbol="X", bool edge=false);
    static void getMaximum(const TMTape3D& tape, int& x, int& y, int& z);
    static void getCentralTop(const TMTape3D& tape, int& x, int& y, int& z);
    static std::string getWaterScriptForTape(const TMTape3D& tape, unsigned int CASizeX = 5, unsigned int CASizeY = 5, unsigned int CASizeZ = 5, int waterSourceX = -1, int waterSourceY = -1, int waterSourceZ = -1);
    static void generateTerrain(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, const double& scale=0.5);
    /**
     * Generates a terrain with 3D Perlin noise
     * @param space output voxels
     * @param x width of the terrain
     * @param y length of the terrain
     * @param z height of the terrain
     * @param scale filling factor (from 0 to 1) for Perlin noise
     */
    static void generateCheese(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, const double& scale=0.5);
    /**
    * Generates a dot visualisation for a Turing machine
    * @tparam TMTapeType types of the tapes (aka their dimensions)
    * @param TM Turing machine to visualise
    * @param path path to the output file
    */
    template<class... TMTapeType>
    static void TMtoDotfile(MTMDTuringMachine<TMTapeType...> TM, const std::string& path) {
        FiniteControl control = TM.getFiniteControl();
        finiteControlToDotfile(control, path);
    }
private:
    static void voxeliseFace(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize, const Vector3D& translationPoint, int start, int end);
};


#endif //VOXELFUSION_UTILS_H
