#include "renderer.hpp"
#include <unordered_map>
#include <memory>
#include "entity.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "light.hpp"
#include "ID.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "ModelLoader.hpp"
#include "light.hpp"

extern unsigned int WINDOW_WIDTH;
extern unsigned int WINDOW_HEIGHT;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

void Renderer::init() {
	// create a default shader
	unique_ptr<Shader> shader = make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
	// std::cout << "Creating Directional Shadow depth shader..." << std::endl;
	unique_ptr<Shader> depth = make_unique<Shader>("shaders/depthShader.vert", "shaders/depthShader.frag");
	depthShader = depth->ID;
	unsigned int depthShaderID = depth->ID;
	shaders[depthShaderID] = move(depth);
	// std::cout << "Creating Point Shadow depth shader..." << std::endl;
	unique_ptr<Shader> point = make_unique<Shader>("shaders/depthPointShader.vert", "shaders/depthPointShader.frag", "shaders/depthPointShader.geom");
	depthPointShader = point->ID;
	unsigned int pointShaderID = point->ID;
	shaders[pointShaderID] = move(point);
	// std::cout << "Creating light shader..." << std::endl;
	unique_ptr<Shader> lightShader = make_unique<Shader>("shaders/meshLights.vert", "shaders/meshLights.frag");
	defaultShader = lightShader->ID;
	unsigned int lightShaderID = lightShader->ID;
	shaders[lightShaderID] = move(lightShader);
	unique_ptr<Shader> lightCube = make_unique<Shader>("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightCubeShader = lightCube->ID;
	unsigned int lightCubeID = lightCube->ID;
	shaders[lightCubeID] = move(lightCube);
	// create a default material
	addMaterial(true);
	// create a default mesh for lightCube
	lightCubeMeshID = addMesh(CUBE);

	// initialize 2d shadow maps for direcitonal lights and spot lights
	glGenFramebuffers(MAX_SHADOW_MAPS, depthMapFBOs);
	glGenTextures(MAX_SHADOW_MAPS, depthMaps);
	for (unsigned int i = 0; i < MAX_SHADOW_MAPS; i++) {
		glBindTexture(GL_TEXTURE_2D, depthMaps[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
			GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, depthMaps[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	// initialize cube maps for point lights
	glGenFramebuffers(MAX_SHADOW_MAPS, depthCubeMapFBOs);
	glGenTextures(MAX_SHADOW_MAPS, depthCubeMaps);
	for (unsigned int i = 0; i < MAX_SHADOW_MAPS; i++) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMaps[i]);
		for (unsigned int j = 0; j < 6; j++) 
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBOs[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMaps[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	//addLight(DIRECTIONAL);
	//addLight(POINT);
	//addLight(SPOT);
}

unsigned int Renderer::addLight(Light_Type type) {
	unsigned int newID = lightID.getID();
	if (type == DIRECTIONAL) {
		lights[newID] = move(make_unique<DirectionalLight>(newID));
	}
	else if (type == POINT) {
		lights[newID] = move(make_unique<PointLight>(newID));
	}
	else if (type == SPOT) {
		lights[newID] = move(make_unique<SpotLight>(newID));
	}
	std::cout << "Light added with ID: " << newID << std::endl;
	return newID;
}

unsigned int Renderer::addShader(const string& vertPath, const string& fragPath, const string& geomPath) {
	unique_ptr<Shader> shader = make_unique<Shader>(vertPath.c_str(), fragPath.c_str(), geomPath.c_str());
	unsigned int newID = shader->ID;
	shaders[newID] = move(shader);
	std::cout << "Shader added with ID: " << newID << std::endl;
	return newID;
}

unsigned int Renderer::addMaterial(bool flag, vector<Texture> texs) {
	unsigned int newID = materialID.getID();
	materials[newID] = make_unique<Material>(newID, flag, defaultShader, texs);
	std::cout << "Material added with ID: " << newID << std::endl;
	return newID;
}

unsigned int Renderer::addEntity(unsigned int meshID, unsigned int matID) {
	unsigned int newID = entityID.getID();
	vector<Component> comps;
	comps.emplace_back(meshID, matID);
	entities[newID] = make_unique<Entity>(newID, comps);
	return newID;
}

unsigned int Renderer::addEntity(Mesh_Type mType, const string& path) {
	unsigned int newID = entityID.getID();
	vector<Component> comps;
	if (mType == OTHER)
		loadModel(path, comps);
	else
		comps.emplace_back(addMesh(mType), 0);
	entities[newID] = make_unique<Entity>(newID, comps);
	std::cout << "Entity added with ID: " << newID << std::endl;
	return newID;
}

unsigned int Renderer::addMesh(Mesh_Type type) {
	unsigned int newID = meshID.getID();
	if (type == CUBE) {
		meshes[newID] = move(make_unique<Cube>(newID));
		std::cout << "Cube added with ID: " << newID << std::endl;
	}
	else if (type == SPHERE)
		meshes[newID] = move(make_unique<Sphere>(newID));

	meshes[newID]->name = "Mesh " + std::to_string(newID);
	std::cout << "Mesh added with ID: " << newID << std::endl;
	return newID;
}

unsigned int Renderer::addMesh(Mesh_Type type, vector<Vertex> initVertices, vector<unsigned int> initIndices) {
	unsigned int newID = meshID.getID();
	meshes[newID] = move(make_unique<Mesh>(newID, initVertices, initIndices));
	std::cout << "Mesh added with ID: " << newID << std::endl;
	return newID;
}

void Renderer::removeEntity(unsigned int eID) {
	// remove entity from the map
	entities.erase(eID);
	// release the ID for reuse
	entityID.releaseID(eID);
	std::cout << "Entity deleted with ID: " << eID << std::endl;
}

void Renderer::removeMaterial(unsigned int mID) {
	// remove material from the map
	materials.erase(mID);
	// release the ID for reuse
	materialID.releaseID(mID);
}

void Renderer::removeLight(unsigned int lID) {
	// unbind the light from the texture unit
	glActiveTexture(GL_TEXTURE0 + lights[lID]->textureUnit);
	glBindTexture(lights[lID]->type == POINT ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
	// remove light from the map
	lights.erase(lID);
	// release the ID for reuse
	lightID.releaseID(lID);
	std::cout << "Light deleted with ID: " << lID << std::endl;
}

void Renderer::deleteMesh(unsigned int mID) {
	// remove mesh from the map
	meshes.erase(mID);
	// release the ID for reuse
	meshID.releaseID(mID);
}

void Renderer::updateLight() {
	unsigned int dirCount = 0, pointCount = 0, spotCount = 0;
	for (auto const& [lID, l] : lights) {
		if (l->type == DIRECTIONAL) {
			l->updateUBO(dirCount++);
		}
		else if (l->type == POINT) {
			l->updateUBO(pointCount++);
		}
		else if (l->type == SPOT) {
			l->updateUBO(spotCount++);
		}
	}
	Light::updateLightNum();
}

void Renderer::render(bool lightVisible) {
	updateLight();
	Shader& depth = *shaders[depthShader];
	Shader& depthPoint = *shaders[depthPointShader];

	unsigned int dirCount = 0, pointCount = 0, spotCount = 0;

	// create shadow maps for each light
	glCullFace(GL_FRONT);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	for (auto const& [lID, l] : lights) {
		if (l->type == POINT) {
			if (pointCount >= MAX_SHADOW_MAPS) {
				continue;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBOs[pointCount]);
			glClear(GL_DEPTH_BUFFER_BIT);

			// setup the light space matrices for the cube map
			depthPoint.use();
			glUniform1f(glGetUniformLocation(depthPointShader, "far_plane"), ((PointLight*)l.get())->far_plane);
			glUniform3fv(glGetUniformLocation(depthPointShader, "lightPos"), 1, glm::value_ptr(l->position));
			for (unsigned int k = 0; k < 6; k++) {
				glUniformMatrix4fv(glGetUniformLocation(depthPointShader, ("lightSpaceMatrices[" + to_string(k) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(l->lightSpaceMatrices[k]));
				//std::cout << "lightSpaceMatrices[" << k << "]: " << glm::to_string(l->lightSpaceMatrices[k]) << std::endl;
			}
			renderScene(true, depthPointShader);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else {
			unsigned int index = 0;
			if (l->type == DIRECTIONAL) {
				if (dirCount >= MAX_SHADOW_MAPS / 2)
					continue;
				index = dirCount++;
			}
			else if (l->type == SPOT) {
				if (spotCount >= MAX_SHADOW_MAPS / 2)
					continue;
				index = spotCount++ + MAX_SHADOW_MAPS / 2;
			}
			// render to the depth map
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[index]);
			glClear(GL_DEPTH_BUFFER_BIT);

			// setup the light space matrix
			depth.use();
			//std::cout << "lightSpaceMatrices[0]: " << glm::to_string(l->lightSpaceMatrices[0]) << std::endl;
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(l->lightSpaceMatrices[0]));

			renderScene(true, depthShader);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	glCullFace(GL_BACK);

	// render the scene
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderScene(false);

	// draw the lights
	Shader &lightCube = *shaders[lightCubeShader];
	for (auto const& [lID, l] : lights) {
		if (l->type != DIRECTIONAL && l->visible) {
			// create a unit cube to represent the light
		
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(l->position));
			glm::mat4 model = translate * scale;

			glUseProgram(lightCubeShader);
			glUniformMatrix4fv(glGetUniformLocation(lightCubeShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

			// draw the light
			Mesh mesh = *meshes[lightCubeMeshID];
			mesh.draw(lightCube);
		}
	}

	// unbind the textures from all units
	//for (auto const& [lID, l] : lights) {
	//	glActiveTexture(GL_TEXTURE0 + l->textureUnit);
	//	glBindTexture(l->type == POINT ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
	//}
	
}

void Renderer::updateShadowMaps(Shader& shader) {
	shader.use();
	unsigned int textureUnitOffset = 0;
	unsigned int dirCount = 0, pointCount = 0, spotCount = 0;

	for (auto const& [lID, l] : lights) {
		glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
		
		// process directional and spot lights
		if (l->type == DIRECTIONAL || l->type == SPOT) {
			unsigned int index = 0;
			if (l->type == DIRECTIONAL) {
				if (dirCount >= MAX_SHADOW_MAPS / 2)
					continue;
				index = dirCount++;
			}
			else if (l->type == SPOT) {
				if (spotCount >= MAX_SHADOW_MAPS / 2)
					continue;
				index = spotCount++ + MAX_SHADOW_MAPS / 2;
			}
			l->textureUnit = textureUnitOffset;
			glBindTexture(GL_TEXTURE_2D, depthMaps[index]);
			glUniform1i(glGetUniformLocation(shader.ID, ("shadowMap[" + to_string(index) + "]").c_str()), textureUnitOffset);
			textureUnitOffset++;
		}
		else if (l->type == POINT) {
			if (pointCount >= MAX_SHADOW_MAPS)
				continue;
			l->textureUnit = textureUnitOffset;
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMaps[pointCount]);
			glUniform1i(glGetUniformLocation(shader.ID, ("shadowCubeMap[" + to_string(pointCount) + "]").c_str()), textureUnitOffset);
			
			textureUnitOffset++;
			pointCount++;
		}
		else {
			std::cout << "Invalid light type" << std::endl;
		}
	}
}

void Renderer::renderScene(bool shadow, unsigned int shaderID, bool lightVisible) {
	for (auto const& [eID, e] : entities) {
		if (e->render) {
			glm::mat4 eModel = getModelMatrix(*e);

			for (auto& comp : e->components) {
				Mesh& mesh = *(meshes[comp.meshID]);
				Material& mat = *(materials[comp.matID]);
				Shader& shader = shadow ? *shaders[shaderID] : *shaders[mat.shaderID];

				if (!shadow) {
					mat.setupUniforms(shader);
					updateShadowMaps(shader);
					
				}

				shader.use();
				// setup the model matrix
				glm::mat4 cModel = getModelMatrix(comp);

				glm::mat4 model = eModel * cModel;

				glUseProgram(shader.ID);
				glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
				mesh.draw(shader);

				//if (!shadow)
				//	mat.unbindTextures();
			}
		}
	}
}