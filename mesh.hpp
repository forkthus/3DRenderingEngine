#pragma once

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <math.h>
#include "shader.hpp"
#include "texture.hpp"

#include <iostream>

using std::string;
using std::vector;

enum Mesh_Type {
	CUBE,
	SPHERE,
	OTHER
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class Mesh {
public:
	// mesh data
	string name;
	vector<Vertex> vertices;
	vector<Texture> textures;
	vector<unsigned int> indices;
	Mesh_Type type;
	glm::vec3 scale;
	unsigned int ID;

	Mesh() = default;

	Mesh(unsigned int id) : ID(id) {
		this->type = OTHER;
		setupMesh();
	}

	Mesh(unsigned int id, vector<Vertex> initVertices, vector<unsigned int> initIndices) : ID(id), vertices(initVertices), indices(initIndices) {
		this->type = OTHER;
		setupMesh();
	}

	virtual ~Mesh() {}

	void setupMesh() {
		name = "Mesh " + std::to_string(ID);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoords));
		
		glBindVertexArray(0);
	}

	void draw(Shader& shader) {
		shader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	virtual glm::mat4 getScaleMatrix() {
		return glm::mat4(1.0f);
	}

protected:
	unsigned int VAO, VBO, EBO;
};

class Sphere : public Mesh {
public:
	int sectorCount;
	int stackCount;

	Sphere(unsigned int id, float r = 1.0f, int sectorCnt = 32, int stackCnt = 32) : sectorCount(sectorCnt), stackCount(stackCnt) {
		ID = id;
		generateSphere();
		setupMesh();
		this->type = SPHERE;
		this->scale = glm::vec3(r, r, r);
	}

	~Sphere() = default;

	void updateSectors() {
		//sectorCount = newSectorCount;
		std::cout<<sectorCount<<std::endl;
		generateSphere();
		setupMesh();
	}

	void updateStacks() {
		//stackCount = newStackCount;
		std::cout << stackCount << std::endl;
		generateSphere();
		setupMesh();
	}

private:
	// function for generate a smooth unit sphere with current sectorCount and stackCount
	void generateSphere() {
		// intialize the vector
		vertices.clear();
		indices.clear();

		float x, y, z, xy;          // vector for vertices
		float s, t;                 // vector for texture coordinates

		float sectorStep = 2 * M_PI / sectorCount;
		float stackStep = M_PI / stackCount;
		float sectorAngle, stackAngle;

		// generate vertices, nromals, and texture coordinates
		for (unsigned int i = 0; i <= stackCount; i++) {
			stackAngle = M_PI / 2 - i * stackStep;
			xy = cosf(stackAngle);
			z = sinf(stackAngle);

			// each add (sectorCount+1) vertices, since the last texture coordinate and  the first texture coordinate should not be the same
			for (unsigned int j = 0; j <= sectorCount; j++) {
				Vertex vert;
				sectorAngle = j * sectorStep;

				// generate vertices coordinates
				x = xy * cosf(sectorAngle);
				y = xy * sinf(sectorAngle);

				vert.position = glm::vec3(x, y, z);

				// since its a normal sphere, the normal is the same as its coordinates
				vert.normal = glm::vec3(x, y, z);

				// generate texture coordinates
				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				vert.textureCoords = glm::vec2(s, t);

				vertices.push_back(vert);
			}
		}

		// generate indices for the sphere
		unsigned int k1, k2;
		for (unsigned int i = 0; i < stackCount; i++) {
			k1 = i * (sectorCount + 1);     // current stack
			k2 = k1 + sectorCount + 1;      // next stack
			for (unsigned int j = 0; j < sectorCount; j++, k1++, k2++) {
				// for the first and the last stack, only one triangle per sector needed to add
				// two triangles per sector otherwise
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (stackCount - 1)) {
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}

			}
		}
	}
};

class Cube : public Mesh {
public:
	Cube(unsigned int id, float l = 1.0f, float w = 1.0f, float h = 1.0f) {
		ID = id;
		generateCube();
		setupMesh();
		this->type = CUBE;
		this->scale = glm::vec3(l, w, h);
	}

	~Cube() = default;

private:
	void generateCube() {
		vector<float> vertex_coords {
			-0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f, 0.5f, -0.5f,
			- 0.5f, 0.5f, -0.5f,
			-0.5f, -0.5f, 0.5f,
			0.5f, -0.5f, 0.5f,
			0.5f, 0.5f, 0.5f,
			-0.5f, 0.5f, 0.5f,
			-0.5f, 0.5f, 0.5f,
			-0.5f, 0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, 0.5f,
			0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, -0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, 0.5f,
			-0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, 0.5f,
			-0.5f, -0.5f, 0.5f,
			-0.5f, 0.5f, -0.5f,
			0.5f, 0.5f, -0.5f,
			0.5f, 0.5f, 0.5f,
			-0.5f, 0.5f, 0.5f 
		};

		vector<float> vertex_normals {
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		};

		vector<float> vertex_texCoords {
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
		};

		indices = {
			0, 2, 1,
			2, 0, 3,
			4, 5, 6,
			6, 7, 4,
			8, 9, 10,
			10, 11, 8,
			12, 14, 13,
			14, 12, 15,
			16, 17, 18,
			18, 19, 16,
			20, 22, 21,
			22, 20, 23
		};
		
		for (int i = 0; i < vertex_coords.size() / 3; i++) {
			Vertex vert;
			vert.position = glm::vec3(vertex_coords[3 * i], vertex_coords[3 * i + 1], vertex_coords[3 * i + 2]);
			vert.normal = glm::vec3(vertex_normals[3 * i], vertex_normals[3 * i + 1], vertex_normals[3 * i + 2]);
			vert.textureCoords = glm::vec2(vertex_texCoords[2 * i], vertex_texCoords[2 * i + 1]);
			
			vertices.push_back(vert);
		} 
	}
};
