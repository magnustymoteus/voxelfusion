//
// Created by daria on 19/11/2023.
//

#include "utils.h"
#include "obj_parser.h"
#include "Mesh.h"
#include "vector/vector3d.h"
#include <fstream>
#include <limits>
#include <algorithm>
#include <cmath>
#include "MTMDTuringMachine/TMTape.h"
#include "PerlinNoise.h"

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


    //-- fast box-plane test
    float r = 0.5 * fabs(norm.x) + 0.5 * fabs(norm.y) + 0.5 * fabs(norm.z);
    float s = norm.x * (0.5f - v[0].x) + norm.y * (0.5f - v[0].y) + norm.z * (0.5f - v[0].z);
    if (fabs(s) > r) return false;

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
    voxelSpace.resize(static_cast<size_t>(std::ceil(maxPoint.x)),
                     std::vector<std::vector<Voxel>>(static_cast<size_t>(std::ceil(maxPoint.y)),
                                                     std::vector<Voxel>(static_cast<size_t>(std::ceil(maxPoint.z)))));
    // Iterate through each face of the mesh and mark the intersected voxels
    for (const auto& face : mesh.faces) {
        for (size_t i = 0; i < face.point_indexes.size(); i += 3) { // Just for safety, normally it should give only one iteration
            // Get the points
            Vector3D v0 = mesh.points[face.point_indexes[i]];
            Vector3D v1 = mesh.points[face.point_indexes[i + 1]];
            Vector3D v2 = mesh.points[face.point_indexes[i + 2]];
            // Translate and scale them
            translateAndScale(v0, translationPoint, voxelSize);
            translateAndScale(v1, translationPoint, voxelSize);
            translateAndScale(v2, translationPoint, voxelSize);

            // Iterate through each voxel
            for(unsigned x = 0; x != voxelSpace.size(); x++){
                for(unsigned y = 0; y != voxelSpace[x].size(); y++){
                    for(unsigned z = 0; z != voxelSpace[x][y].size(); z++){
                        // Check if the voxel intersects the current triangle
//                        if(x == 3 && y==3){
//                            std::cout << std::endl;
//                        }
                        if(voxelTriangleIntersection(x,y,z,v0,v1,v2)) voxelSpace[x][y][z].occupied = true;
                    }
                }
            }
        }
    }
}

void utils::voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol, bool edge){
    if(!edge) { // Yes, the code is almost the same, but otherwise it would be a mess
        for (unsigned int x = 0; x < voxelSpace.size(); x++) {
            TMTape2D TMPlane;
            for (unsigned int y = 0; y < voxelSpace[x].size(); y++) {
                TMTape1D TMLine;
                for (unsigned int z = 0; z < voxelSpace[x][y].size(); z++) {
                    std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                    TMLine[z] = TMTapeCell(symbol);
                }
                TMPlane[y] = TMLine;
            }
            tape[x] = TMPlane;
        }
    } else{
        // TODO: remove redundant code + do 2n+1 resize
        tape.cells.resize(voxelSpace.size()+2);
        //////////// top begin
        TMTape2D zeroplane;
        zeroplane.cells.resize(voxelSpace[0].size()+2);

        TMTape1D line00;
        line00.cells.resize(voxelSpace[0][0].size()+2);
        for (unsigned int z = 0; z < voxelSpace[0][0].size()+2; z++) {
            line00[z] = TMTapeCell("BB");
        }
        zeroplane[0] = line00;

        for (unsigned int y = 0; y < voxelSpace[0].size(); y++) {
            TMTape1D TMLine;
            TMLine.cells.resize(voxelSpace[0][y].size()+2);
            for (unsigned int z = 0; z < voxelSpace[0][y].size()+2; z++) {
                TMLine.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
            }
            zeroplane.cells[y+1] = std::make_shared<TMTape1D>(TMLine);
        }

        TMTape1D lastline0;
        lastline0.cells.resize(voxelSpace[0][0].size()+2);
        for (unsigned int z = 0; z < voxelSpace[0][0].size()+2; z++) {
            lastline0.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
        }
        zeroplane.cells[zeroplane.cells.size()-1] = std::make_shared<TMTape1D>(lastline0);

        tape.cells[0] = std::make_shared<TMTape2D>(zeroplane);
        ////////////// top end

        // Here we construct planes:
        for (unsigned int x = 0; x < voxelSpace.size(); x++) {
            TMTape2D TMPlane;
            TMPlane.cells.resize(voxelSpace[x].size()+2);
            // left size begin
            TMTape1D line0;
            line0.cells.resize(voxelSpace[x][0].size()+2);
            for (unsigned int z = 0; z < voxelSpace[x][0].size()+2; z++) {
                line0.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
            }
            TMPlane.cells[0] = std::make_shared<TMTape1D>(line0);
            // left size end
            // Here we construct lines for the plane:
            for (unsigned int y = 0; y < voxelSpace[x].size(); y++) {
                TMTape1D TMLine;
                TMLine.cells.resize(voxelSpace[x][y].size()+2);
                TMLine.cells[0] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
                for (unsigned int z = 0; z < voxelSpace[x][y].size(); z++) {
                    std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                    TMLine.cells[z+1] = std::make_shared<TMTapeCell>(TMTapeCell(symbol));
                }
                TMLine.cells[voxelSpace[x][y].size()+1] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
                TMPlane.cells[y+1] = std::make_shared<TMTape1D>(TMLine);
            }
            // right size begin
            TMTape1D lastline;
            lastline.cells.resize(voxelSpace[x][0].size()+2);
            for (unsigned int z = 0; z < voxelSpace[x][0].size()+2; z++) {
                lastline.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
            }
            TMPlane.cells[voxelSpace[x][0].size()+1] = std::make_shared<TMTape1D>(lastline);
            // right size end
            tape.cells[x+1] = std::make_shared<TMTape2D>(TMPlane);
        }
        //////////// bottom begin
        TMTape2D lastplane;
        lastplane.cells.resize(voxelSpace[0].size()+2);

        TMTape1D linelast0;
        linelast0.cells.resize(voxelSpace[0][0].size()+2);
        for (unsigned int z = 0; z < voxelSpace[0][0].size()+2; z++) {
            linelast0.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
        }
        lastplane.cells[0] = std::make_shared<TMTape1D>(linelast0);

        for (unsigned int y = 0; y < voxelSpace[0].size(); y++) {
            TMTape1D TMLine;
            TMLine.cells.resize(voxelSpace[0][y].size()+2);
            for (unsigned int z = 0; z < voxelSpace[0][y].size()+2; z++) {
                TMLine.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
            }
            lastplane.cells[y+1] = std::make_shared<TMTape1D>(TMLine);
        }

        TMTape1D linelastlast;
        linelastlast.cells.resize(voxelSpace[0][0].size()+2);
        for (unsigned int z = 0; z < voxelSpace[0][0].size()+2; z++) {
            linelastlast.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell("BB"));
        }
        lastplane[lastplane.cells.size()-1] = linelastlast;

        tape[voxelSpace.size()+1] = lastplane;
        /////////// bottom end
    }
}

void utils::generateTerrain(VoxelSpace& space, const unsigned int& xi, const unsigned int& yi, const unsigned int& zi, const double& scale){
        space.resize(static_cast<size_t>(xi),
                      std::vector<std::vector<Voxel>>(static_cast<size_t>(yi),
                                                      std::vector<Voxel>(zi)));
    for(unsigned x = 0; x != xi; x++){
        for(unsigned y=0; y != yi; y++){
            for(unsigned z=0; z != zi; z++){
                PerlinNoise p;
                double noise = p.noise3d(x*scale,y*scale,z*scale);
                if(noise >= 0) space[x][y][z].occupied = true;
            }
        }
    }
}