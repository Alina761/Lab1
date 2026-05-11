#pragma once
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include "Mesh.h"

class Model {
public:
    Model(const char* path);
    void Draw(unsigned int shaderProgram);
    void DrawPart(unsigned int index, unsigned int shaderProgram, const glm::mat4& modelMatrix);

private:
    std::vector<Mesh> meshes;
    std::string directory;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};