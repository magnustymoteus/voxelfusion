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
void utils::voxelise(const Mesh& mesh, VoxelSpace& voxelSpace, float voxelSize){
    BoundingBox bbox = calculateBoundingBox(mesh); // The box won't be shifted!

    // Calculate the dimensions of the voxel space based on the mesh bounding box
    float gridSizeX = (bbox.maxX - bbox.minX) / voxelSize;
    float gridSizeY = (bbox.maxY - bbox.minY) / voxelSize;
    float gridSizeZ = (bbox.maxZ - bbox.minZ) / voxelSize;

    // Calculate the offset for each dimension
    double xOffset = bbox.minX > 0 ? 0 : -bbox.minX;
    double yOffset = bbox.minY > 0 ? 0 : -bbox.minY;
    double zOffset = bbox.minZ > 0 ? 0 : -bbox.minZ;
    Vector3D translationPoint = Vector3D::point(-xOffset, -yOffset, -zOffset);

    // Initialize the voxel space
    voxelSpace.resize(static_cast<size_t>(gridSizeX),
                     std::vector<std::vector<Voxel>>(static_cast<size_t>(gridSizeY),
                                                     std::vector<Voxel>(static_cast<size_t>(gridSizeZ))));
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

            // Perform voxelisation for the current triangle
             drawTriangle(v0, v1, v2, voxelSpace, voxelSize);
        }
    }
}

void utils::drawTriangle(const Vector3D& a, const Vector3D& b, const Vector3D& c, VoxelSpace& voxelSpace, float voxelSize){
    // Pixels to triangle
    double y_max = std::max(a.y, std::max(b.y, c.y));
    double y_min = std::min(a.y, std::min(b.y, c.y));
    // Calculate xg, yg, zg
    double xg = (a.x + b.x + c.x)/3.0;
    double yg = (a.y + b.y + c.y)/3.0;
    double zg = (1.0/(3.0*a.z)) + (1.0/(3.0*b.z)) + (1.0/(3.0*c.z));
    // TODO: check if it rounds properly
    for(int y_i = static_cast<int>(std::lround(y_min)); y_i <= y_max; y_i++){
        //double y_i = static_cast<double> (y_ik);
        double x_lab = std::numeric_limits<double>::infinity();
        double x_lac = std::numeric_limits<double>::infinity();
        double x_lbc = std::numeric_limits<double>::infinity();
        double x_rab = -std::numeric_limits<double>::infinity();
        double x_rac = -std::numeric_limits<double>::infinity();
        double x_rbc = -std::numeric_limits<double>::infinity();
        // There are 3 cases
        // AB
        if((y_i - a.y)*(y_i - b.y) <= 0 && a.y != b.y){
            double x_i = b.x + ((a.x - b.x)*(y_i - b.y)/(a.y - b.y));
            x_lab = x_i;
            x_rab = x_i;
        }
        // BC
        if((y_i - b.y)*(y_i - c.y) <= 0 && b.y != c.y){
            double x_i = c.x + ((b.x - c.x)*(y_i - c.y)/(b.y - c.y));
            x_lbc = x_i;
            x_rbc = x_i;
        }
        // AC
        if((y_i - a.y)*(y_i - c.y) <= 0 && a.y != c.y){
            double x_i = c.x + ((a.x - c.x)*(y_i - c.y)/(a.y - c.y));
            x_lac = x_i;
            x_rac = x_i;
        }

        // Draw everything line per line
        if(std::round(std::min(x_lab, std::min(x_lac, x_lbc))) != std::numeric_limits<double>::infinity() &&
           std::round(std::max(x_rab, std::max(x_rac, x_rbc))) != -std::numeric_limits<double>::infinity()) {
            int x_r = static_cast<int>(std::lround(std::min(x_lab, std::min(x_lac, x_lbc)) + 0.5));
            int x_l = static_cast<int>(std::lround(std::max(x_rab, std::max(x_rac, x_rbc)) - 0.5));
            double Z = zg + (x_r-xg)*voxelSize + (y_i-yg)*voxelSize;

            for (int x = x_r; x <= x_l; x++) {
                if(x >= static_cast<int>(voxelSpace.size())) continue;
                if(y_i >= static_cast<int>(voxelSpace[x].size())) continue;
                if(static_cast<int>(std::floor(1/Z)) >= static_cast<int>(voxelSpace[x][y_i].size())) continue;
                // TODO: do the proper voxel operation(s) + check 1/Z
                int z = static_cast<int>(std::floor(1/Z)); // for debug purposes
                voxelSpace[x][y_i][z].occupied = true;
                Z += voxelSize; // TODO: also check the logic here
            }

        }
    }
}

void utils::voxelSpaceToTape(const VoxelSpace& voxelSpace, TMTape3D& tape, const std::string& fillSymbol){
    tape.cells.resize(voxelSpace.size());
    for(unsigned int x = 0; x < voxelSpace.size(); x++){
        TMTape2D TMPlane;
        TMPlane.cells.resize(voxelSpace[x].size());
        for(unsigned int y = 0; y < voxelSpace[x].size(); y++){
            TMTape1D TMLine;
            TMLine.cells.resize(voxelSpace[x][y].size());
            for(unsigned int z = 0; z < voxelSpace[x][y].size(); z++){
                //std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                std::string symbol = voxelSpace[x][y][z].occupied ? fillSymbol : "B";
                TMLine.cells[z] = std::make_shared<TMTapeCell>(TMTapeCell(symbol));
            }
            TMPlane.cells[y] = std::make_shared<TMTape1D>(TMLine);
        }
        tape.cells[x] = std::make_shared<TMTape2D>(TMPlane);
    }
}