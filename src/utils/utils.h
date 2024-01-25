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

// Voxels
using VoxelSpace = std::vector<std::vector<std::vector<Voxel>>>;
using CompletedVoxelSpace = std::vector<std::vector<std::vector<std::string>>>;

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
    static void bmpToTasm(const std::string& pathToImage, const std::string& pathToTasm);
    static void voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize=1);
    /**
     * Put the given voxel space on the tape
     * @param voxelSpace input voxel space
     * @param tape output tape
     * @param fillSymbol symbol of the not empty tape cells
     * @param edge if true, generates invisible bounding box (filled with "BB" symbol) around the tape
     */
    static void voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol="X", bool edge=false);
    static void completedVoxelSpaceToTape(const CompletedVoxelSpace& voxelSpace, TMTape3D& tape);
    static void objToTape(const std::string& path, TMTape3D& tape, const double& voxelSize=1, const std::string& fillSymbol="X", bool edge=false);
    static void tapeToCompletedVoxelSpace(const TMTape3D& tape, CompletedVoxelSpace& voxelSpace);
    static void save3DTapeToJson(const TMTape3D& tape, std::string outputPath="savedTape.json");
    static void load3DTapeFromJson(TMTape3D& tape, std::string inputPath="savedTape.json");
    static void getMaximum(const TMTape3D tape, int& x, int& y, int& z);
    static void getCentralTop(const TMTape3D& tape, int& x, int& y, int& z);
    /**
     * Generates tasm script and place a source of the water. If coordinates of the water source are not given, it chooses the
     * @param tape tape of the simulation
     * @param numberOfSteps (default = 10) number of simulation steps
     * @param CASizeX (default = 5) x-range of the CA
     * @param CASizeY (default = 5) y-range of the CA
     * @param CASizeZ (default = 5) z-range of the CA
     * @param waterSourceX x-coordinate of the water
     * @param waterSourceY y-coordinate of the water
     * @param waterSourceZ z-coordinate of the water
     * @return tasm script for water simulation
     */
    static std::string getWaterScriptForTape(TMTape3D& tape, unsigned int numberOfSteps = 10, unsigned int CASizeX = 5, unsigned int CASizeY = 5, unsigned int CASizeZ = 5, int waterSourceX = -1, int waterSourceY = -1, int waterSourceZ = -1);
    /**
     * Generates a terrain with 2D positive Perlin noise (without cylinder equation)
     * @param space output voxels
     * @param x width of the terrain
     * @param y length of the terrain
     * @param z height of the terrain
     * @param random true if the gradient is randomly chosen (default false)
     * @param scale filling factor (from 0 to 1) for Perlin noise.
     */
    static void generateTerrain(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, bool random = false, const double& scale=0.1);
    /**
     * Generates a terrain with classical 2D Perlin noise (and with cylinder equation)
     * @param space output voxels
     * @param x width of the terrain
     * @param y length of the terrain
     * @param z height of the terrain
     * @param random true if the gradient is randomly chosen (default false)
     * @param scale filling factor (from 0 to 1) for Perlin noise.
     */
    static void generateTerrain2(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, bool random = false, const double& scale=0.1);
    /**
     * Generates a terrain with 3D Perlin noise
     * @param space output voxels
     * @param x width of the terrain
     * @param y length of the terrain
     * @param z height of the terrain
     * @param random true if the gradient is randomly chosen (default false)
     * @param scale filling factor (from 0 to 1) for Perlin noise.
     */
    static void generateCheese(VoxelSpace& space, const unsigned int& x, const unsigned int& y, const unsigned int& z, bool random = false, const double& scale=0.1);
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
