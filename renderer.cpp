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
	shaders[depthShader] = move(depth);
	// std::cout << "Creating Point Shadow depth shader..." << std::endl;
	unique_ptr<Shader> point = make_unique<Shader>("shaders/depthPointShader.vert", "shaders/depthPointShader.frag", "shaders/depthPointShader.geom");
	depthPointShader = point->ID;
	shaders[depthPointShader] = move(point);
	// std::cout << "Creating light shader..." << std::endl;
	unique_ptr<Shader> lightShader = make_unique<Shader>("shaders/meshLights.vert", "shaders/meshLights.frag");
	defaultShader = lightShader->ID;
	shaders[defaultShader] = move(lightShader);
	unique_ptr<Shader> lightCube = make_unique<Shader>("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightCubeShader = lightCube->ID;
	shaders[lightCubeShader] = move(lightCube);
	unique_ptr<Shader> skybox = make_unique<Shader>("shaders/skybox.vert", "shaders/skybox.frag");
	skyboxShader = skybox->ID;
	shaders[skyboxShader] = move(skybox);
	unique_ptr<Shader> hightlight = make_unique<Shader>("shaders/highlight.vert", "shaders/highlight.frag");
	highlightShader = hightlight->ID;
	shaders[highlightShader] = move(hightlight);
	unique_ptr<Shader> geometryPass = make_unique<Shader>("shaders/deferredShadingGeometry.vert", "shaders/deferredShadingGeometry.frag");
	geometryPassShader = geometryPass->ID;
	shaders[geometryPassShader] = move(geometryPass);
	unique_ptr<Shader> lightingPass = make_unique<Shader>("shaders/deferredShadingLighting.vert", "shaders/deferredShadingLighting.frag");
	lightingPassShader = lightingPass->ID;
	shaders[lightingPassShader] = move(lightingPass);
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

	// setup framebuffer for deferred rendering
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);		// 16 bit per channel for higher precision
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);		// 16 bit per channel for higher precision
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// albedo + specular buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);		// 8 bit per channel
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// setup depth buffer for framebuffer
	glGenRenderbuffers(1, &renderDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderDepthBuffer);
	
	// setup stencil buffer for framebuffer
	//glGenRenderbuffers(1, &renderStencilBuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, renderStencilBuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderStencilBuffer);
	
	// check the completeness of framebuffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer for deferred rendering is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	// setup screen quad
	float quadVertices[] = {
		// positions		// texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	// setup Skybox
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//setupSkybox({ "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" });
	//setupSkybox({ "textures/tf_right.png", "textures/tf_left.png", "textures/tf_top.png", "textures/tf_bottom.png", "textures/tf_front.png", "textures/tf_back.png" });
	setupSkybox({ "textures/bluecloud_rt.jpg", "textures/bluecloud_lf.jpg", "textures/bluecloud_up.jpg", "textures/bluecloud_dn.jpg", "textures/bluecloud_ft.jpg", "textures/bluecloud_bk.jpg" });
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
	unique_ptr<Shader> shader = make_unique<Shader>(vertPath.c_str(), fragPath.c_str(), geomPath.empty() ? nullptr : geomPath.c_str());
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
	Shader& geometryPass = *shaders[geometryPassShader];
	Shader& lightingPass = *shaders[lightingPassShader];

	unsigned int dirCount = 0, pointCount = 0, spotCount = 0;

	// create shadow maps for each light
	glCullFace(GL_FRONT);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	// create shadow maps for each light
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//renderSkyBox();

	// deferred rendering
	bool deferred = true;
	if (deferred) {
		// geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		renderScene(false, geometryPassShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// lighting pass
		lightingPass.use();
		glActiveTexture(GL_TEXTURE27);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE28);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE29);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		// setup uniforms for lighting pass
		// updateShadowMaps(lightingPass);
		glUniform1i(glGetUniformLocation(lightingPassShader, "gPosition"), 27);
		glUniform1i(glGetUniformLocation(lightingPassShader, "gNormal"), 28);
		glUniform1i(glGetUniformLocation(lightingPassShader, "gAlbedoSpec"), 29);
		renderQuad();

		// copy depth and stencil buffer to default framebuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else {
		// forward rendering
		renderScene(false);
	}
	renderHighlightObjs();
	renderSkyBox();
	
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
	Shader& highlight = *shaders[highlightShader];
	for (auto const& [eID, e] : entities) {
		if (e->render) {
			glm::mat4 eModel = getModelMatrix(*e);

			for (auto& comp : e->components) {
				Mesh& mesh = *(meshes[comp.meshID]);
				Material& mat = *(materials[comp.matID]);
				Shader& shader = (shaderID != 0) ? *shaders[shaderID] : *shaders[mat.shaderID];

				if (!shadow) {
					// std::cout << "Setting up uniforms\n" << std::endl;
					mat.setupUniforms(shader);
					updateShadowMaps(shader);
					if (e->showProperties) {
						glStencilFunc(GL_ALWAYS, 1, 0xFF);
						glStencilMask(0xFF);
					}
				}

				shader.use();
				// setup the model matrix
				glm::mat4 cModel = getModelMatrix(comp);

				glm::mat4 model = eModel * cModel;

				glUseProgram(shader.ID);
				glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
				mesh.draw(shader);
			}
		}
	}
}

void Renderer::setupSkybox(vector<string> images) {
	skyboxTexture = make_unique<Texture>(TEXTURE_CUBE_MAP, images);
	std::cout << "Loading finished" << std::endl;
}

void Renderer::renderSkyBox() {
	glDepthFunc(GL_LEQUAL);
	Shader& skybox = *shaders[skyboxShader];

	skybox.use();
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture->ID);
	glUniform1i(glGetUniformLocation(skybox.ID, "skybox"), 30);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Renderer::renderHighlightObjs() {
	Shader& shader = *shaders[highlightShader];
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	for (auto const& [eID, e] : entities) {
		glm::mat4 eModel = getModelMatrix(*e);
		if (e-> render && e->showProperties) {
			for (auto& comp : e->components) {
				glm::mat4 cModel = getModelMatrix(comp);

				glm::mat4 model = eModel * cModel;
				glUseProgram(highlightShader);
				glUniformMatrix4fv(glGetUniformLocation(highlightShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
				(meshes[comp.meshID])->draw(*shaders[highlightShader]);
			}
			
		}
	}
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::renderQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}