#pragma once

#define MAX_NUM_TEXTURES 10

#include <string>
#include <vector>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "texture.hpp"

using namespace std;

constexpr size_t VEC4_SIZE = 4 * sizeof(float);

class Material {
public:
	// common
	unsigned int ID;
	string name;
	float shininess;
	unsigned int isColor;		// if true, then the material is a color, else it is a collection of textures
	bool showProperties;
	// textures
	vector<Texture> textures;
	unsigned int shaderID;
	queue<unsigned int> textureUnits;
	float heightScale;

	// color
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	Material() = default;

	Material(unsigned int id, bool iscolor, unsigned int sh, vector<Texture> texs = {}) : ID(id), isColor(iscolor), shaderID(sh), textures(texs) {
		ambient = glm::vec3(0.0215f, 0.1745f, 0.0215f);
		diffuse = glm::vec3(0.07568f, 0.61424f, 0.07568f);
		specular = glm::vec3(0.633f, 0.727811f, 0.633f);
		showProperties = false;
		shininess = 32.0f;
		heightScale = 0.1f;
		name = "Material " + to_string(id);
	}

	static void init();
	
	// should only be run before the rendering the object
	void setupUniforms(Shader &shader);

	void addTexture(Texture_Type, string&);

	void unbindTextures();
	
private:
	static unsigned int UBO;
};
