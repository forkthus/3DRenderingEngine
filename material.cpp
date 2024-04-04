#include "material.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "light.hpp"

unsigned int Material::UBO;

void Material::init() {
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 5 * VEC4_SIZE, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// should only be run before the rendering the object
void Material::setupUniforms(Shader& shader) {
	unsigned int diffuseCount = 0;
	unsigned int specularCount = 0;
	unsigned int normalCount = 0;
	unsigned int heightCount = 0;

	unsigned int offset = 10;

	shader.use();
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &shininess);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float), sizeof(unsigned int), &isColor);

	if (isColor == 0) {
		for (unsigned int i = 0; i < textures.size(); i++) {
			unsigned int num = 0;
			string typeName;
			Texture_Type textureType = textures[i].type;
			// std::cout << "Start binding" << std::endl;
			if (textureType == TEXTURE_DIFFUSE) {
				// std::cout << "diffuse texture found" << std::endl;
				num = diffuseCount++;
				typeName = "texture_diffuse";
				// std::cout << "number is " << num << std::endl;
			}
			else if (textureType == TEXTURE_SPECULAR) {
				// std::cout << "specular texture found" << std::endl;
				num = specularCount++;
				typeName = "texture_specular";
			}
			else if (textureType == TEXTURE_NORMAL) {
				num = normalCount++;
				typeName = "texture_normal";
			}
			else if (textureType == TEXTURE_HEIGHT) {
				num = heightCount++;
				typeName = "texture_height";
			}
			else
				std::cerr << "TEXTURE TYPE FAILURE" << std::endl;

			if (num > MAX_NUM_TEXTURES) {
				std::cerr << "TOO MANY TEXTURES OF TYPE " << typeName << std::endl;
				continue;
			}
			// std::cout << "Binding " << typeName << "[" << to_string(num) << "] to texture " << i << std::endl;
			glActiveTexture(GL_TEXTURE0 + offset + i);
			glUniform1i(glGetUniformLocation(shader.ID, (typeName + "[" + to_string(num) + "]").c_str()), offset + i);
			//std::cout << "Binding " << typeName << "[" << to_string(num) << "] to texture " << i << std::endl;
			//std::cout << "Texture ID is " << textures[i].ID << std::endl;
			glBindTexture(GL_TEXTURE_2D, textures[i].ID);
			
			//textureUnits.push(offset + i);
		}
	}
	else {
		glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 1, sizeof(glm::vec3), glm::value_ptr(ambient));
		glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 2, sizeof(glm::vec3), glm::value_ptr(diffuse));
		glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 3, sizeof(glm::vec3), glm::value_ptr(specular));
	}
	glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 4 + sizeof(unsigned int) * 0, sizeof(unsigned int), &diffuseCount);
	glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 4 + sizeof(unsigned int) * 1, sizeof(unsigned int), &specularCount);
	glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 4 + sizeof(unsigned int) * 2, sizeof(unsigned int), &normalCount);
	glBufferSubData(GL_UNIFORM_BUFFER, VEC4_SIZE * 4 + sizeof(unsigned int) * 3, sizeof(unsigned int), &heightCount);
	glUniform1f(glGetUniformLocation(shader.ID, string("heightScale").c_str()), heightScale);
	glUniform1f(glGetUniformLocation(shader.ID, string("minLayers").c_str()), minLayers);
	glUniform1f(glGetUniformLocation(shader.ID, string("maxLayers").c_str()), maxLayers);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glActiveTexture(GL_TEXTURE0);
}

void Material::addTexture(Texture_Type type, string& path) {
	textures.emplace_back(type, path, path);
}

//void Material::unbindTextures() {
//	if (isColor)	return;
//	while (!textureUnits.empty()) {
//		glActiveTexture(GL_TEXTURE10 + textureUnits.front());
//		glBindTexture(GL_TEXTURE_2D, 0);
//		textureUnits.pop();
//	}
//
//}