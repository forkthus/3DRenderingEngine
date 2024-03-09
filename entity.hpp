#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include <vector>
#include <queue>
#include <math.h>
#include <iostream>
#include <memory>

#include "material.hpp"
#include "mesh.hpp"
#include "renderer.hpp"

using std::unique_ptr;

struct Vertex;

struct Component {
	unsigned int meshID;
	unsigned int matID;
	glm::vec3 pos;
	glm::vec3 scale;
	glm::vec3 rotation;

	Component(unsigned int mesh, unsigned int mat, 
		glm::vec3 p = glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3 s = glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3 r = glm::vec3(0.0f, 0.0f, 0.0f)) : meshID(mesh), matID(mat), pos(p), scale(s), rotation(r) {}

	glm::mat4 getRotationMatrix() {
		glm::quat quatX = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::quat quatY = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::quat quatZ = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1));

		return glm::toMat4(glm::normalize(quatZ * quatY * quatX));
	}
};

extern Renderer rs;

class Entity {
public:
	// data
    unsigned int ID;
    string name;
	vector<Component> components;

	// transform
	glm::vec3 pos;
	glm::vec3 scale;
	glm::vec3 rotation;
	glm::quat globalOrientation;
	glm::quat localOrientation;

	// GUI properties
    bool showProperties;
    bool render;
	Component* selectedComponent;

	Entity(unsigned int id, vector<Component> comps) : ID(id), components(comps) {
		pos = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		showProperties = false;
		globalOrientation = glm::quat(1, 0, 0, 0);
		localOrientation = glm::quat(1, 0, 0, 0);
		render = true;
		selectedComponent = &components[0];

		name = "Entity " + std::to_string(ID);
	}

	glm::mat4 getRotationMatrix() {
		glm::quat quatX = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::quat quatY = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::quat quatZ = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1));

		return glm::toMat4(glm::normalize(quatZ * quatY * quatX));
	}
};

template<typename T>
glm::mat4 getModelMatrix(T& e) {
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), e.pos);
	glm::mat4 rotate = e.getRotationMatrix();
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), e.scale);

	return translate * rotate * scale;
}
