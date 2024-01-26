//
// Created by daria on 19/11/2023.
//

#include "utils.h"
#include "OBJParser/OBJParser.h"
#include "obj_parser.h"
#include "Mesh.h"
#include "vector/vector3d.h"
#include <fstream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <regex>
#include <thread>
#include <string>
#include "MTMDTuringMachine/TMTape.h"
#include "PerlinNoise.h"
#include "json.hpp"

using json = nlohmann::json;

void utils::load_obj(const std::string& path, Mesh& mesh){
    // Get the object
    obj::OBJFile obj_parser;
    std::ifstream input_stream(path);
    input_stream >> obj_parser;
    input_stream.close();
    obj::ObjectGroup object = obj_parser.get_object();

    // Get mtl file name
    std::string mtl_file = object.get_mtllib_file_name();

    // Create new figure
    Mesh figure;

    // Get points coordinates
    std::vector<std::vector<double>> ObjPoints =  object.get_vertexes();
    // Set points
    for(std::vector<double> coos: ObjPoints){
        Vector3D point = Vector3D::point(coos[0], coos[1], coos[2]);
        figure.points.push_back(point);
    }

    // Get polygons (aka super faces)
    std::vector<obj::Polygon> polygons = object.get_polygons();
    // Get all uv coordinates
    std::vector<std::vector<double>> allUVs = object.get_texture_coordinates();
    // Get all normal vectors' coordinates
    std::vector<std::vector<double>> allNorms = object.get_vertex_normals();

    // Transform polygons in conventional faces
    for(obj::Polygon polygon: polygons){
        std::vector<int> indexes = polygon.get_indexes();
        for(int &index:indexes){
            index--;
        }
        Face face = Face(indexes);
        figure.faces.push_back(face);
    }
    mesh = figure;
}

void utils::load_obj2(const std::string& path, Mesh& mesh){
    mesh = OBJParser::parse(path);
}

// Function to calculate the bounding box of a mesh
BoundingBox utils::calculateBoundingBox(const Mesh& mesh) {
    BoundingBox bbox;
    bbox.minX = bbox.minY = bbox.minZ = std::numeric_limits<float>::max();
    bbox.maxX = bbox.maxY = bbox.maxZ = std::numeric_limits<float>::min();

    for (const auto& vertex : mesh.points) {
        bbox.minX = std::min(bbox.minX, vertex.x);
        bbox.minY = std::min(bbox.minY, vertex.y);
        bbox.minZ = std::min(bbox.minZ, vertex.z);

        bbox.maxX = std::max(bbox.maxX, vertex.x);
        bbox.maxY = std::max(bbox.maxY, vertex.y);
        bbox.maxZ = std::max(bbox.maxZ, vertex.z);
    }

    return bbox;
}
void utils::translateAndScale(Vector3D& point, const Vector3D& translatePoint, double scaler){
    point -= translatePoint;
    point /= scaler;
}
bool utils::voxelTriangleIntersection(const int& x, const int& y, const int& z, const Vector3D& v0, const Vector3D& v1, const Vector3D& v2){
    // Inspirited by https://github.com/ramakarl/voxelizer/blob/master/math_voxelizer/main_voxelizer.cpp
    // and https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/pubs/tribox.pdf

    Vector3D v[3], e[3], norm;
    Vector3D p, n;
    float min, max, rad;

    Vector3D c = Vector3D::point(x+0.5, y+0.5,z+0.5);
    // triangle normalized to test cube
    v[0] = v0 - c;
    v[1] = v1 - c;
    v[2] = v2 - c;
    e[0] = v[1] - v[0];	// triangle edges
    e[1] = v[2] - v[1];
    e[2] = v[0] - v[2];
    norm = Vector3D::cross(e[0], e[1]);
    norm.normalise();


//    //-- fast box-plane test
//    float r = 0.5 * fabs(norm.x) + 0.5 * fabs(norm.y) + 0.5 * fabs(norm.z);
//    float s = norm.x * (0.5f - v[0].x) + norm.y * (0.5f - v[0].y) + norm.z * (0.5f - v[0].z);
//    if (fabs(s) > r) return false;

    //--- akenine-moller tests
    p.x = e[0].z * v[0].y - e[0].y * v[0].z;
    p.z = e[0].z * v[2].y - e[0].y * v[2].z;
    if (p.x < p.z) { min = p.x; max = p.z; }
    else { min = p.z; max = p.x; }
    rad = fabsf(e[0].z) * 0.5f + fabsf(e[0].y) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = -e[0].z * v[0].x + e[0].x * v[0].z;
    p.z = -e[0].z * v[2].x + e[0].x * v[2].z;
    if (p.x < p.z) { min = p.x; max = p.z; }
    else { min = p.z; max = p.x; }
    rad = fabsf(e[0].z) * 0.5f + fabsf(e[0].x) * 0.5f; if (min > rad || max < -rad) return false;

    p.y = e[0].y * v[1].x - e[0].x * v[1].y;
    p.z = e[0].y * v[2].x - e[0].x * v[2].y;
    if (p.z < p.y) { min = p.z; max = p.y; }
    else { min = p.y; max = p.z; }
    rad = fabsf(e[0].y) * 0.5f + fabsf(e[0].x) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = e[1].z * v[0].y - e[1].y * v[0].z;
    p.z = e[1].z * v[2].y - e[1].y * v[2].z;
    if (p.x < p.z) { min = p.x; max = p.z; }
    else { min = p.z; max = p.x; }
    rad = fabsf(e[1].z) * 0.5f + fabsf(e[1].y) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = -e[1].z * v[0].x + e[1].x * v[0].z;
    p.z = -e[1].z * v[2].x + e[1].x * v[2].z;
    if (p.x < p.z) { min = p.x; max = p.z; }
    else { min = p.z; max = p.x; }
    rad = fabsf(e[1].z) * 0.5f + fabsf(e[1].x) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = e[1].y * v[0].x - e[1].x * v[0].y;
    p.y = e[1].y * v[1].x - e[1].x * v[1].y;
    if (p.x < p.y) { min = p.x; max = p.y; }
    else { min = p.y; max = p.x; }
    rad = fabsf(e[1].y) * 0.5f + fabsf(e[1].x) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = e[2].z * v[0].y - e[2].y * v[0].z;
    p.y = e[2].z * v[1].y - e[2].y * v[1].z;
    if (p.x < p.y) { min = p.x; max = p.y; }
    else { min = p.y; max = p.x; }
    rad = fabsf(e[2].z) * 0.5f + fabsf(e[2].y) * 0.5f; if (min > rad || max < -rad) return false;

    p.x = -e[2].z * v[0].x + e[2].x * v[0].z;
    p.y = -e[2].z * v[1].x + e[2].x * v[1].z;
    if (p.x < p.y) { min = p.x; max = p.y; }
    else { min = p.y; max = p.x; }
    rad = fabsf(e[2].z) * 0.5f + fabsf(e[2].x) * 0.5f; if (min > rad || max < -rad) return false;

    p.y = e[2].y * v[1].x - e[2].x * v[1].y;
    p.z = e[2].y * v[2].x - e[2].x * v[2].y;
    if (p.z < p.y) { min = p.z; max = p.y; }
    else { min = p.y; max = p.z; }
    rad = fabsf(e[2].y) * 0.5f + fabsf(e[2].x) * 0.5f; if (min > rad || max < -rad) return false;
    return true;
}
void utils::voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize){
    BoundingBox bbox = calculateBoundingBox(mesh); // The box won't be shifted!

    // Calculate the offset for each dimension
    double xOffset = -bbox.minX;
    double yOffset = -bbox.minY;
    double zOffset = -bbox.minZ;
    Vector3D translationPoint = Vector3D::point(-xOffset, -yOffset, -zOffset);
    // Get the max point in the new system of the coordinates
    Vector3D maxPoint = Vector3D::point(bbox.maxX, bbox.maxY, bbox.maxZ);
    translateAndScale(maxPoint, translationPoint, voxelSize);

    // Initialize the voxel space
    voxelSpace.resize(static_cast<size_t>(std::ceil(maxPoint.x)+1),
                     std::vector<std::vector<Voxel>>(static_cast<size_t>(std::ceil(maxPoint.y)+1),
                                                     std::vector<Voxel>(static_cast<size_t>(std::ceil(maxPoint.z)+1))));
    // Calculate optimal number of threads
    const int num_threads = std::thread::hardware_concurrency();
    // Get number of elements
    const int num_elements = mesh.faces.size();
    // Thread vector
    std::vector<std::thread> threads;
    // Chunk size
    int chunk_size = num_elements / num_threads;
    // Start index
    int start = 0;
    static int processed = 0;
    for (int i = 0; i < num_threads - 1; ++i) {
        int end = start + chunk_size;
        threads.emplace_back(voxeliseFace, std::ref(mesh), std::ref(voxelSpace), std::ref(voxelSize), std::ref(translationPoint), start, end);
        start = end;
    }

    // The last thread handles the remaining elements
    threads.emplace_back(voxeliseFace, std::ref(mesh), std::ref(voxelSpace), std::ref(voxelSize), std::ref(translationPoint), start, num_elements);
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
}

void utils::voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol, bool edge){
    unsigned int counter = 0;
    if(!edge) { // Yes, the code is almost the same, but otherwise it would be a mess
        for (unsigned int x = 0; x < voxelSpace.size(); x++) {
            TMTape2D TMPlane;
            for (unsigned int y = 0; y < voxelSpace[x].size(); y++) {
                TMTape1D TMLine;
                for (unsigned int z = 0; z < voxelSpace[x][y].size(); z++) {
                    if (voxelSpace[x][y][z].occupied) counter++;
                    std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                    TMLine[z] = TMTapeCell(symbol);
                }
                TMPlane[y] = TMLine;
            }
            tape[x] = TMPlane;
        }
    } else{ // BB is a "bound blanco"
        // Add extra top plane
        TMTape2D topPlane;
        for (unsigned int y = 0; y < voxelSpace[0].size(); y++) {
            TMTape1D TMLine;
            TMLine[-1] = TMTapeCell("BB");
            for (unsigned int z = 0; z < voxelSpace[0][y].size(); z++) {
                std::string symbol = "BB";
                TMLine[z] = TMTapeCell(symbol);
            }
            TMLine[voxelSpace[0][y].size()] = TMTapeCell("BB");
            topPlane[y] = TMLine;
        }
        tape[-1] = topPlane;
        // Top plane end

        for (unsigned int x = 0; x < voxelSpace.size(); x++) {
            TMTape2D TMPlane;
            // Left line
            TMTape1D leftLine;
            for (unsigned int z = 0; z < voxelSpace[0][0].size(); z++) {
                std::string symbol = "BB";
                leftLine[z] = TMTapeCell(symbol);
            }
            TMPlane[-1] = leftLine;
            // Left line end
            // Reading the information in the vector
            for (unsigned int y = 0; y < voxelSpace[x].size(); y++) {
                TMTape1D TMLine;
                // Forward plane
                TMLine[-1] = TMTapeCell("BB");
                for (unsigned int z = 0; z < voxelSpace[x][y].size(); z++) {
                    if (voxelSpace[x][y][z].occupied) counter++;
                    std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                    TMLine[z] = TMTapeCell(symbol);
                }
                // Back plane
                TMLine[voxelSpace[x][y].size()] = TMTapeCell("BB");
                TMPlane[y] = TMLine;
            }
            // End information reading
            // Right line
            TMTape1D rightLine;
            for (unsigned int z = 0; z < voxelSpace[0][0].size(); z++) {
                std::string symbol = "BB";
                rightLine[z] = TMTapeCell(symbol);
            }
            TMPlane[voxelSpace[x].size()] = rightLine;
            // Right line end
            tape[x] = TMPlane;
        }


        // Add extra bottom plane
        TMTape2D bottomPlane;
        for (unsigned int y = 0; y < voxelSpace[0].size(); y++) {
            TMTape1D TMLine;
            TMLine[-1] = TMTapeCell("BB");
            for (unsigned int z = 0; z < voxelSpace[0][y].size(); z++) {
                std::string symbol = "BB";
                TMLine[z] = TMTapeCell(symbol);
            }
            TMLine[voxelSpace[0][y].size()] = TMTapeCell("BB");
            bottomPlane[y] = TMLine;
        }
        tape[voxelSpace.size()] = bottomPlane;
        // Bottom plane end
    }
    std::cout << "Filled blocks in voxelSpaceToTape: " << counter << std::endl;
}
void utils::completedVoxelSpaceToTape(const CompletedVoxelSpace &voxelSpace, TMTape3D &tape){
    for (unsigned int x = 0; x < voxelSpace.size(); x++) {
        TMTape2D TMPlane;
        for (unsigned int y = 0; y < voxelSpace[x].size(); y++) {
            TMTape1D TMLine;
            for (unsigned int z = 0; z < voxelSpace[x][y].size(); z++) {
                TMLine[z] = TMTapeCell(voxelSpace[x][y][z]);
            }
            TMPlane[y] = TMLine;
        }
        tape[x] = TMPlane;
    }
}
void utils::generateTerrain(VoxelSpace& space, const unsigned int& xi, const unsigned int& yi, const unsigned int& zi, bool random, const double& scale){
    space.resize(static_cast<size_t>(xi),
                 std::vector<std::vector<Voxel>>(static_cast<size_t>(zi),
                                                 std::vector<Voxel>(yi)));
    PerlinNoise p(random);
    for(unsigned x = 0; x != xi; x++){
        for(unsigned y = 0; y != yi; y++){
            double P = yi*(p.positiveNoise2d(scale*x,scale*y));
            int height = P + 1;
            //std::cout << "Height: " << height << ", P: " << P << std::endl;
            for(auto f = 0; f != height && f != zi ; f++){
                space[x][f][y].occupied = true;
            }
        }
    }
}

void utils::generateTerrain2(VoxelSpace& space, const unsigned int& xi, const unsigned int& yi, const unsigned int& zi, bool random, const double& scale){
        space.resize(static_cast<size_t>(xi),
                      std::vector<std::vector<Voxel>>(static_cast<size_t>(zi),
                                                      std::vector<Voxel>(yi)));
    PerlinNoise p(random);
    for(unsigned x = 0; x != xi; x++){
        for(unsigned y = 0; y != yi; y++){
            double H = 0.4*xi - 0.02*(pow(x-xi/2.0,2)+pow(y-yi/2.0,2));
            double P = 0.5*yi*(-0.5 + p.noise2d(scale*x,scale*y));
            int height = std::max(0,int(H+P))+1;
            //std::cout << height << ", H: " << H << ", P: " << P << std::endl;
            for(auto f = 0; f != height && f != zi ; f++){
                space[x][f][y].occupied = true;
            }
        }
    }
}

void utils::generateCheese(VoxelSpace& space, const unsigned int& xi, const unsigned int& yi, const unsigned int& zi, bool random, const double& scale){
    space.resize(static_cast<size_t>(xi),
                 std::vector<std::vector<Voxel>>(static_cast<size_t>(yi),
                                                 std::vector<Voxel>(zi)));
    PerlinNoise p(random);
    for(unsigned x = 0; x != xi; x++){
        for(unsigned y = 0; y != yi; y++){
            for(unsigned z = 0; z != zi; z++){
                double noise = p.noise3d(x*scale,y*scale,z*scale);
                if(noise < 0) space[x][y][z].occupied = true;
            }
        }
    }
}

void utils::finiteControlToDotfile(FiniteControl &control, const std::string &path) {
   std::string result = "DiGraph G {\n";
    for(auto &transition: control.transitions){
        for(auto &stateTransition : transition.second) {
            result += transition.first;
            result += "->";
            std::string replaced = std::accumulate(stateTransition.first.begin(), stateTransition.first.end(), std::string(""));
            std::string replacedBy = std::accumulate(stateTransition.second.replacementSymbols.begin(), stateTransition.second.replacementSymbols.end(), std::string(""));
            std::string direction;
            for(auto& dir: stateTransition.second.directions){
                // here we assume that no probability between directions is involved (every direction has 100% chance)
                char a = static_cast<char>(dir());
                direction.push_back(a);
                direction += " | ";
            }
            result += stateTransition.second.state->name + "[label=\"" + replaced + '\\' + "/" + replacedBy + ", " + direction + "\"]" + '\n';
        }
        }
    result += "}";
    std::ofstream output(path);
    output << result;
    output.close();
}

void utils::objToTape(const std::string& path, TMTape3D& tape, const double& voxelSize, const std::string& fillSymbol, bool edge){
    Mesh mesh;
    VoxelSpace voxelSpace;
    load_obj2(path, mesh);
    voxelise(mesh, voxelSpace, voxelSize);
    voxelSpaceToTape(voxelSpace, tape, fillSymbol, edge);
}

void utils::voxeliseFace(const Mesh& mesh, VoxelSpace& voxelSpace, double voxelSize, const Vector3D& translationPoint, int start, int end) {
    for(int f = start; f != end; ++f) {
        // Get the points
        Vector3D v0 = mesh.points[mesh.faces[f].point_indexes[0]];
        Vector3D v1 = mesh.points[mesh.faces[f].point_indexes[1]];
        Vector3D v2 = mesh.points[mesh.faces[f].point_indexes[2]];
        // Translate and scale them
        translateAndScale(v0, translationPoint, voxelSize);
        translateAndScale(v1, translationPoint, voxelSize);
        translateAndScale(v2, translationPoint, voxelSize);

        // Iterate through each voxel
        for (unsigned x = 0; x != voxelSpace.size(); x++) {
            for (unsigned y = 0; y != voxelSpace[x].size(); y++) {
                for (unsigned z = 0; z != voxelSpace[x][y].size(); z++) {
                    if (voxelTriangleIntersection(x, y, z, v0, v1, v2)) voxelSpace[x][y][z].occupied = true;
                }
            }
        }
    }
}

void utils::getMaximum(TMTape3D tape, int &x, int &y, int &z){
    x = std::floor(tape.getCells().size()/2.0);
    y = std::floor(tape.getElementSize()/2.0);
    z = std::floor(tape.at(x).getElementSize()/2.0);
}

void utils::getCentralTop(const TMTape3D &tape, int &x, int &y, int &z) {
    getMaximum(tape, x, y, z);
    if(tape.zeroAnchor != 0) x /= 2;
    if(tape.zeroAnchor != 0) y = std::ceil(tape.at(x).getCells().size()/4.0);
    else y = std::ceil(tape.at(x).getCells().size());
    if(tape.zeroAnchor != 0) z = std::floor(tape.at(x).at(y-1).getCells().size()/4.0)-1;
    else z = std::floor(tape.at(x).at(y-1).getCells().size()/2.0)-1;
}

std::string utils::getWaterScriptForTape(TMTape3D& tape, unsigned int numberOfSteps, unsigned int CASizeX, unsigned int CASizeY, unsigned int CASizeZ, int waterSourceX, int waterSourceY, int waterSourceZ){
    // Step 1: read tasm template code
    std::string code;
    std::string line;
    std::ifstream input ("tasm/water-physics-template.tasm");
    if (input.is_open())
    {
        while (getline (input, line))
        {
            code += line;
        }
        input.close();
    }
    // Step 2: get a good position for water source (if the position is not given)
    if(waterSourceX < 0 || waterSourceY < 0 || waterSourceZ < 0){
        getCentralTop(tape, waterSourceX, waterSourceY, waterSourceZ);
        // Step 3: Replace the macros
        code = std::regex_replace(code, std::regex("#CA_X_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceX - (CASizeX/2)))));
        code = std::regex_replace(code, std::regex("#CA_Y_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceY - CASizeY + 2))));
        code = std::regex_replace(code, std::regex("#CA_Z_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceZ - (CASizeZ/2)))));
        // Step 4: place the water source
        tape[waterSourceX][waterSourceY][waterSourceZ].symbol = "W";
    }else{
        // Step 3: Replace the macros
        code = std::regex_replace(code, std::regex("#CA_X_POSITION"), std::to_string(waterSourceX));
        code = std::regex_replace(code, std::regex("#CA_Y_POSITION"), std::to_string(waterSourceY));
        code = std::regex_replace(code, std::regex("#CA_Z_POSITION"), std::to_string(waterSourceZ));
        // Step 4: place the water source
        tape[waterSourceX][waterSourceY][waterSourceZ].symbol = "W";
    }
    // Step 5: replace other macros
    code = std::regex_replace(code, std::regex("#CA_X_SIZE"), std::to_string(CASizeX));
    code = std::regex_replace(code, std::regex("#CA_Y_SIZE"), std::to_string(CASizeY));
    code = std::regex_replace(code, std::regex("#CA_Z_SIZE"), std::to_string(CASizeZ));
    code = std::regex_replace(code, std::regex("#NUMBER_OF_STEPS"), std::to_string(numberOfSteps));

    // Step 6: return the code
    return code;
}
std::string utils::getBoomScriptForTape(TMTape3D& tape, unsigned int numberOfSteps, unsigned int CASizeX, unsigned int CASizeY, unsigned int CASizeZ, int waterSourceX, int waterSourceY, int waterSourceZ){
    // Step 1: read tasm template code
    std::string code;
    std::string line;
    std::ifstream input ("tasm/boom-template.tasm");
    if (input.is_open())
    {
        while (getline (input, line))
        {
            code += line;
        }
        input.close();
    }
    // Step 2: get a good position for water source (if the position is not given)
    if(waterSourceX < 0 || waterSourceY < 0 || waterSourceZ < 0){
        getCentralTop(tape, waterSourceX, waterSourceY, waterSourceZ);
        // Step 3: Replace the macros
        code = std::regex_replace(code, std::regex("#CA_X_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceX - (CASizeX/2)))));
        code = std::regex_replace(code, std::regex("#CA_Y_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceY - CASizeY + 2))));
        code = std::regex_replace(code, std::regex("#CA_Z_POSITION"), std::to_string(std::max(0, static_cast<int>(waterSourceZ - (CASizeZ/2)))));
        // Step 4: place the water source
        tape[waterSourceX][waterSourceY][waterSourceZ].symbol = "red";
    }else{
        // Step 3: Replace the macros
        code = std::regex_replace(code, std::regex("#CA_X_POSITION"), std::to_string(waterSourceX));
        code = std::regex_replace(code, std::regex("#CA_Y_POSITION"), std::to_string(waterSourceY));
        code = std::regex_replace(code, std::regex("#CA_Z_POSITION"), std::to_string(waterSourceZ));
        // Step 4: place the water source
        tape[waterSourceX][waterSourceY][waterSourceZ].symbol = "red";
    }
    // Step 5: replace other macros
    code = std::regex_replace(code, std::regex("#CA_X_SIZE"), std::to_string(CASizeX));
    code = std::regex_replace(code, std::regex("#CA_Y_SIZE"), std::to_string(CASizeY));
    code = std::regex_replace(code, std::regex("#CA_Z_SIZE"), std::to_string(CASizeZ));
    code = std::regex_replace(code, std::regex("#NUMBER_OF_STEPS"), std::to_string(numberOfSteps));

    // Step 6: return the code
    return code;
}
void utils::tapeToCompletedVoxelSpace(const TMTape3D& tape, CompletedVoxelSpace& voxelSpace){
    CompletedVoxelSpace toReturn;
    for(auto& plane:tape.getCells()){
        std::vector<std::vector<std::string>> planeStrings;
        for(auto& row:plane->getCells()){
            // Make a vector
            std::vector<std::string> rowStrings;
            for(auto& cell: row->getCells()){
                rowStrings.push_back(cell->symbol);
            }
            planeStrings.push_back(rowStrings);
        }
        toReturn.push_back(planeStrings);
    }
    voxelSpace = toReturn;
}
void utils::save3DTapeToJson(const TMTape3D& tape, std::string outputPath){
    // Transform tape to CompletedVoxelSpace
    CompletedVoxelSpace space;
    tapeToCompletedVoxelSpace(tape, space);
    json jsonRepresentation = space;
    // Save the json to a file
    std::ofstream outputFile(outputPath);
    outputFile << std::setw(4) << jsonRepresentation << std::endl;
    outputFile.close();
}

// Function to get the bit depth of a BMP image
int getBMPBitDepth(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return -1; // Return -1 to indicate an error
    }

    // Skip to the 15th byte in the header (bit depth information)
    file.seekg(14 + 1); // BMP header is 14 bytes, and the bit depth is at the 15th byte

    // Read the bit depth (a 16-bit integer)
    short bitDepth;
    file.read(reinterpret_cast<char*>(&bitDepth), sizeof(bitDepth));

    file.close();

    return bitDepth;
}
// Function to read a bitmap image
bool readBitmap(std::string filename, unsigned char*& imageData, int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    // Read bitmap file header
    char header[54];
    file.read(header, sizeof(header));

    // Extract width and height from the header
    width = *(int*)&header[18];
    height = *(int*)&header[22];

    // Calculate the size of the image data
    int dataSize = width * height * 3; // Assuming 24 bits per pixel (3 bytes)

    // Allocate memory for image data
    imageData = new unsigned char[dataSize];

    // Read image data
    file.read(reinterpret_cast<char*>(imageData), dataSize);

    file.close();

    return true;
}

void utils::bmpToTasm(const std::string& fileName, const std::string& pathToTasm){
    unsigned char* imageData;
    int width, height;

    if (!readBitmap(fileName, imageData, width, height)) {
        throw std::runtime_error("File not found");
    }

    int depth = getBMPBitDepth(fileName)/8;
    // Iterate over pixels
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Calculate the index for the current pixel
            int index = (y * width + x) * depth;

            // Access RGB values for the current pixel
            int red = imageData[index];
            int green = imageData[index + 1];
            int blue = imageData[index + 2];

            // Process or manipulate pixel values as needed
            // For example, print pixel values
            std::cout << "Pixel at (" << x << ", " << y << "): R=" << (int)red << " G=" << (int)green << " B=" << (int)blue << std::endl;
        }
    }
    delete[] imageData;
}

void utils::load3DTapeFromJson(TMTape3D& tape, std::string inputPath){
    // Read the json back into a 3D vector
    std::ifstream inputFile(inputPath);
    json loadedJson;
    inputFile >> loadedJson;

    // Convert the json back to the original voxel space
    CompletedVoxelSpace space = loadedJson;
    // Transform to tape
    completedVoxelSpaceToTape(space, tape);
}