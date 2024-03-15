#pragma once

#include "ID.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

enum Light_Type;
enum Mesh_Type;

class Entity;
class Material;
class Mesh;
class Light;
class Shader;
class Texture;

struct Vertex;
struct Vertex;
struct Component;
 
using std::unordered_map, std::unique_ptr, std::make_unique, std::move, std::string, std::vector;

const unsigned int MAX_SHADOW_MAPS = 10;

class Renderer {
public:
	unordered_map<unsigned int, unique_ptr<Entity>> entities;
	unordered_map<unsigned int, unique_ptr<Material>> materials;
	unordered_map<unsigned int, unique_ptr<Mesh>> meshes;
	unordered_map<unsigned int, unique_ptr<Light>> lights;
	unordered_map<unsigned int, unique_ptr<Shader>> shaders;

	Renderer() = default;

	void init();

	unsigned int addLight(Light_Type type);

	unsigned int addShader(const string& vertPath, const string& fragPath, const string& geomPath = "");

	unsigned int addMaterial(bool flag, vector<Texture> texs = {});

	unsigned int addEntity(unsigned int meshID, unsigned int matID);

	unsigned int addEntity(Mesh_Type mType, const string& path = "");
	
	unsigned int addMesh(Mesh_Type type);

	unsigned int addMesh(Mesh_Type type, vector<Vertex> initVertices, vector<unsigned int> initIndices);

	void removeEntity(unsigned int eID);

	void removeMaterial(unsigned int mID);

	void removeLight(unsigned int lID);

	void deleteMesh(unsigned int mID);

	void updateLight();

	void render(bool lightVisible = false);

	void updateShadowMaps(Shader& shader);

	void renderScene(bool shadow, unsigned int shaderID = 0, bool lightVisible = false);

	void renderSkyBox();

	void setupSkybox(vector<string> images);

private:
	unsigned int defaultShader;
	unsigned int depthShader;
	unsigned int depthPointShader;
	unsigned int lightCubeShader;
	unsigned int lightCubeMeshID;
	unsigned int skyboxShader;
	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
	unique_ptr<Texture> skyboxTexture;
	ID entityID;
	ID materialID;
	ID meshID;
	ID lightID;
	unsigned int depthMapFBOs[MAX_SHADOW_MAPS];
	unsigned int depthMaps[MAX_SHADOW_MAPS];
	unsigned int depthCubeMapFBOs[MAX_SHADOW_MAPS];
	unsigned int depthCubeMaps[MAX_SHADOW_MAPS];
};
