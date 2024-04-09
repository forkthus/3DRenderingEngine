#pragma once

#include "ID.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "glm/glm.hpp"

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

	inline void removeMesh(unsigned int mID);

	void render(bool lightVisible = false);

	void setupSkybox(vector<string> images);

private:
	void updateLight();

	void updateShadowMaps(Shader& shader);

	void renderScene(bool deferred, bool shadow, unsigned int shaderID = 0, bool lightVisible = false);

	void renderSkyBox();

	void renderHighlightObjs();

	inline void renderQuad();

	inline void initSSAO();

	inline void initSkybox();

	inline void initGBuffer();

	inline void initShaders();

	inline float lerp(float a, float b, float f);

	// default shaders
	unsigned int defaultShader;
	unsigned int highlightShader;	// highling the outline of the object

	// lighting cube for visible lights
	unsigned int lightCubeShader;
	unsigned int lightCubeMeshID;

	// skybox
	unsigned int skyboxShader;
	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
	unique_ptr<Texture> skyboxTexture;

	// deferred rendering
	unsigned int gBuffer;
	unsigned int renderDepthBuffer;
	unsigned int renderStencilBuffer;
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoSpec;
	unsigned int geometryPassColoredShader;
	unsigned int geometryPassTexturedShader;
	unsigned int lightingPassShader;
	unsigned int quadVAO;
	unsigned int quadVBO;

	// SSAO
	unsigned int SSAOshader;
	unsigned int SSAOblurShader;
	unsigned int SSAOnoiseTexture;
	unsigned int SSAOfbo;
	unsigned int SSAOblurFBO;
	unsigned int SSAOcolorBuffer;
	unsigned int SSAOblurBuffer;
	vector<glm::vec3> SSAOkernel;

	// IDs
	ID entityID;
	ID materialID;
	ID meshID;
	ID lightID;

	// shadow mapping
	unsigned int depthShader;
	unsigned int depthPointShader;
	unsigned int depthMapFBOs[MAX_SHADOW_MAPS];
	unsigned int depthMaps[MAX_SHADOW_MAPS];
	unsigned int depthCubeMapFBOs[MAX_SHADOW_MAPS];
	unsigned int depthCubeMaps[MAX_SHADOW_MAPS];
};
