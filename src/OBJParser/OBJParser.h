//

#ifndef VOXELFUSION_OBJPARSER_H
#define VOXELFUSION_OBJPARSER_H

#include "vector/vector3d.h"
#include "utils/Mesh.h"

/**
 * @brief .obj file parser that can parse the mesh
 */
class OBJParser {
public:
    static Mesh parse(const std::string objPath);
};


#endif //VOXELFUSION_OBJPARSER_H
