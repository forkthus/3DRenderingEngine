#pragma once

#define MAX_NUM_LIGHTS 128

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
#include <vector>

#include <string>

#include "shader.hpp"
using namespace std;

using std::string, std::vector;

extern const unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
const float aspect_ratio = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

enum Light_Type {
	DIRECTIONAL,
	POINT,
	SPOT
};

struct Light_Component {
	glm::vec3 ambient;
	float padding0;
	glm::vec3 diffuse;
	float padding1;
	glm::vec3 specular;
	float padding2;

	Light_Component() : ambient(0.0f), diffuse(0.0f), specular(0.0f), padding0(0.0f), padding1(0.0f), padding2(0.0f) {}
	Light_Component(glm::vec3 a, glm::vec3 d, glm::vec3 s) : ambient(a), diffuse(d), specular(s), padding0(0.0f), padding1(0.0f), padding2(0.0f) {}
};

struct Attenuation {
	float constant;
	float linear;
	float quadratic;
};

// define the size of each light structs in the UBO
constexpr size_t dirLightSize = 128;
constexpr size_t pointLightSize = 80;
constexpr size_t spotLightSize = 176;
constexpr size_t offset = 16;
constexpr size_t dOffset = offset;
constexpr size_t pOffset = offset + MAX_NUM_LIGHTS * dirLightSize;
constexpr size_t sOffset = offset + MAX_NUM_LIGHTS * dirLightSize + MAX_NUM_LIGHTS * pointLightSize;

class Light {
public:
	glm::vec3 position;
	Light_Type type;
	unsigned int index;
	string name;
	bool showProperties;
	bool visible;
	vector<glm::mat4> lightSpaceMatrices;
	unsigned int textureUnit;

	// count the number of each type of light
	static unsigned int dirLightNum;
	static unsigned int pointLightNum;
	static unsigned int spotLightNum;

	Light_Component lightComponent;

	virtual ~Light() {}

	static void init();

	virtual void updateUBO(unsigned int index) = 0;

	static void updateLightNum() {
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned int), &dirLightNum);
		glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(unsigned int), sizeof(unsigned int), &pointLightNum);
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(unsigned int), sizeof(unsigned int), &spotLightNum);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

protected:  
	static unsigned int UBO;
};

class DirectionalLight : public Light {
public:
	glm::vec3 direction;

	DirectionalLight(unsigned int lightIndex,
		glm::vec3 initDir=glm::vec3(-0.2f, -1.0f, -0.3f), 
		Light_Component initLC=Light_Component(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f))) : direction(initDir) {
		lightComponent = initLC;
		type = DIRECTIONAL;
		index = lightIndex;
		showProperties = false;
		name = "Directional Light "  + std::to_string(index);
		lightSpaceMatrices.push_back(glm::mat4(0.0f));

		dirLightNum++;
		//updateUBO();
	}

	~DirectionalLight() {

		dirLightNum--;
	}

	inline void updateUBO(unsigned int index) {
		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 view = glm::lookAt(-direction, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrices[0] = proj * view;
		// update UBO
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, dOffset + index * dirLightSize, sizeof(glm::vec3), glm::value_ptr(direction));
		glBufferSubData(GL_UNIFORM_BUFFER, dOffset + index * dirLightSize + sizeof(glm::vec4), sizeof(Light_Component), &lightComponent);
		glBufferSubData(GL_UNIFORM_BUFFER, dOffset + index * dirLightSize + 4 * sizeof(glm::vec4), sizeof(glm::mat4), glm::value_ptr(lightSpaceMatrices[0]));
		// std::cout << "lightSpaceMatrix: " << glm::to_string(lightSpaceMatrix) << std::endl;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};

class PointLight : public Light {
public:
	Attenuation attenuation;
	float far_plane;

	PointLight(unsigned int lightIndex, 
		glm::vec3 initPos=glm::vec3(0.0f, 1.0f, 0.0f), 
		Light_Component initLC=Light_Component(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)), 
		Attenuation initAtt = { 1.0f, 0.09f, 0.032f }) : attenuation(initAtt) {
		position = initPos;
		lightComponent = initLC;
		type = POINT;
		index = lightIndex;
		showProperties = false;
		name = "Point Light " + std::to_string(index);
		far_plane = 25.0f;

		float near_plane = 1.0f;
		glm::mat4 proj = glm::perspective(glm::radians(90.0f), aspect_ratio, near_plane, far_plane);
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		lightSpaceMatrices.push_back(proj * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		pointLightNum++;
		//updateUBO();
	}

	~PointLight() {
		pointLightNum--;
	}                                                                                                                                                                                                                                                             

	void updateUBO(unsigned int index) {
		// update UBO
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, pOffset + index * pointLightSize, sizeof(glm::vec3), glm::value_ptr(position));
		glBufferSubData(GL_UNIFORM_BUFFER, pOffset + index * pointLightSize + sizeof(glm::vec4), sizeof(Attenuation), &attenuation);
		glBufferSubData(GL_UNIFORM_BUFFER, pOffset + index * pointLightSize + sizeof(glm::vec4) + 3 * sizeof(float), sizeof(float), &far_plane);
		glBufferSubData(GL_UNIFORM_BUFFER, pOffset + index * pointLightSize + 2 * sizeof(glm::vec4), sizeof(Light_Component), &lightComponent);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};

class SpotLight : public Light {
public:
	glm::vec3 direction;
	// smooth edges
	float cutOff;
	float outerCutOff;
	// attenuation
	Attenuation attenuation;

	SpotLight(unsigned int lightindex, glm::vec3 initPos=glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 initDirec=glm::vec3(0.0f, 0.0f, 1.0f), 
		float initCutOff=glm::cos(glm::radians(12.5f)), float initOuterCutOff=glm::cos(glm::radians(17.5f)), 
		Light_Component initLC=Light_Component(glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f)), 
		Attenuation initAtt={1.0f, 0.09f, 0.032f})
		: direction(initDirec), cutOff(initCutOff), outerCutOff(initOuterCutOff), attenuation(initAtt) {
		position = initPos;
		lightComponent = initLC;
		type = SPOT;
		index = lightindex;
		showProperties = false;
		name = "Spot Light " + std::to_string(index);
		lightSpaceMatrices.push_back(glm::mat4(0.0f));
		
		spotLightNum++;
		//updateUBO();
	}

	~SpotLight() {
		spotLightNum--;
	}

	inline void updateUBO(unsigned int index) {
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 proj = glm::perspective(glm::acos(cutOff) * 2.0f, aspect_ratio, near_plane, far_plane);
		glm::mat4 view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrices[0] = proj * view;
		// update UBO
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize, sizeof(glm::vec3), glm::value_ptr(position));
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(direction));
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + 2 * sizeof(glm::vec4), sizeof(float), &cutOff);
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + 2 * sizeof(glm::vec4) + sizeof(float), sizeof(float), &outerCutOff);
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + 3 * sizeof(glm::vec3), sizeof(Attenuation), &attenuation);
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + 4 * sizeof(glm::vec3), sizeof(Light_Component), &lightComponent);
		glBufferSubData(GL_UNIFORM_BUFFER, sOffset + index * spotLightSize + 7 * sizeof(glm::vec4), sizeof(glm::mat4), glm::value_ptr(lightSpaceMatrices[0]));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};