#include "Model.h"
#include <cstdio>

Model::Model(const char* path) {
    loadModel(path);
}

void Model::Draw(unsigned int shaderProgram) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shaderProgram, glm::mat4(1.0f));
    }
}

void Model::DrawPart(unsigned int index, unsigned int shaderProgram, const glm::mat4& modelMatrix) {
    if (index < meshes.size()) {
        meshes[index].Draw(shaderProgram, modelMatrix);
    }
    else {
        printf("Ошибка: индекс меша %d вне диапазона (всего мешей: %d)\n", index, (int)meshes.size());
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("Ошибка ASSIMP: %s\n", importer.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    printf("Модель загружена: %s, количество мешей: %d\n", path.c_str(), (int)meshes.size());
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->HasNormals()) {
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh(vertices, indices);
}