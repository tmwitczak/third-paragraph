#ifndef MODEL_H
#define MODEL_H
// //////////////////////////////////////////////////////////// Includes //
#include "shader.hpp"
#include "mesh.hpp"

#include "assimp/scene.h"

#include <string>
#include <vector>

// ?
unsigned int TextureFromFile(const char *path,
const std::string &directory, bool gamma = false);


// //////////////////////////////////////////////////////// Class: Model //
class Model {
private:
    std::vector<Mesh> meshes;
    bool gammaCorrection;

public:
    Model(std::string const &path, bool gamma = false);

    void render(Shader &shader);
    
private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat,
            aiTextureType type, std::string typeName);
};


#endif