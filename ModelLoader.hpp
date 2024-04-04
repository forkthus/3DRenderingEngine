#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <utility>
#include "texture.hpp"
#include "entity.hpp"

struct Component;
enum Texture_Type;

using std::pair;

void loadModel(string const& path, vector<Component>& comps);

void processNode(aiNode* node, const aiScene* scene, vector<Component>& comps, const glm::mat4&);

pair<unsigned int, unsigned int> processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4&);

vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, Texture_Type typeName);
