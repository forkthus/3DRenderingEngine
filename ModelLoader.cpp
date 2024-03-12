#include "ModelLoader.hpp"
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include "mesh.hpp"
#include <unordered_map>
#include "renderer.hpp"
#include <filesystem>

using std::unordered_map, std::pair, std::make_pair;

extern Renderer rs;

string directory;
unordered_map<string, Texture> textureMap;

void loadModel(string const& path, vector<Component>& comps) {
	std::cout << "Begin Loading..." << std::endl;
	// initialization
	directory.clear();
	textureMap.clear();
	// read model via Assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/\\'));
	// process Assimp's root node recursively
	std::cout << "Processing Node..." << std::endl;
	processNode(scene->mRootNode, scene, comps);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(aiNode* node, const aiScene* scene, vector<Component>& comps)
{
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		auto ids = processMesh(mesh, scene);
		comps.emplace_back(ids.first, ids.second);
	}
	// std::cout << "Mesh processing finished" << std::endl;
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, comps);
	}
	// std::cout << "Process child nodes" << std::endl;
}

pair<unsigned int, unsigned int> processMesh(aiMesh* mesh, const aiScene* scene) {
	std::cout << "Processing Mesh..." << std::endl;
	// data to fill
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vect;

		// positions
		vect.x = mesh->mVertices[i].x;
		vect.y = mesh->mVertices[i].y;
		vect.z = mesh->mVertices[i].z;
		vertex.position = vect;

		// normals
		if (mesh->HasNormals()) {
			vect.x = mesh->mNormals[i].x;
			vect.y = mesh->mNormals[i].y;
			vect.z = mesh->mNormals[i].z;
			vertex.normal = vect;
		}

		// tangents
		vect.x = mesh->mTangents[i].x;
		vect.y = mesh->mTangents[i].y;
		vect.z = mesh->mTangents[i].z;
		vertex.tangent = vect;

		// bitangents
		vect.x = mesh->mBitangents[i].x;
		vect.y = mesh->mBitangents[i].y;
		vect.z = mesh->mBitangents[i].z;
		vertex.bitangent = vect;

		// texture coordinates
		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.textureCoords = vec;
			// tangent
			vect.x = mesh->mTangents[i].x;
			vect.y = mesh->mTangents[i].y;
			vect.z = mesh->mTangents[i].z;
			vertex.tangent = vect;
			// bitangent
			vect.x = mesh->mBitangents[i].x;
			vect.y = mesh->mBitangents[i].y;
			vect.z = mesh->mBitangents[i].z;
			vertex.bitangent = vect;
		}
		else
			vertex.textureCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	std::cout << "Vertices processed" << std::endl;
	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices oairpods maxf the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE);
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 2. specular maps
	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR);
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT);
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	std::cout << "Textures processed" << std::endl;
	unsigned int meshID = rs.addMesh(OTHER, vertices, indices);
	std::cout << "Mesh ID: " << meshID << std::endl;
	unsigned int matID = rs.addMaterial(false, textures);
	return make_pair(meshID, matID);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, Texture_Type typeName) {
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		if (textureMap.find(str.C_Str()) != textureMap.end()) {
			textures.push_back(textureMap[str.C_Str()]);	
		}
		else
		{   // if texture hasn't been loaded already, load it
			std::filesystem::path texturePath = std::filesystem::path(directory) / str.C_Str();
			Texture texture(typeName, texturePath.string(), texturePath.string());
			textures.push_back(texture);
			textureMap[str.C_Str()] = texture;
		}
	}
	return textures;
}