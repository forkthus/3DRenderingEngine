#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include <string>
#include <iostream>

using std::string;

Texture::Texture(Texture_Type tType, string tPath, string tName) : name(tName), path(tPath), type(tType) {
	// load image
	int width, height, nrComponents;
	glActiveTexture(GL_TEXTURE31);
	glGenTextures(1, &ID);

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	//std::cout << "texture path: " << path << "\n";
	// setup OpenGL texture
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	size_t lastSlashPos = name.find_last_of("/\\");
	if (lastSlashPos != std::string::npos) {
		// Extract the file name
		name = name.substr(lastSlashPos + 1);
	}
}

Texture::Texture(Texture_Type tType, vector<string> paths, string tName) : name(tName), type(tType) {
	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE30);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	int width, height, nrComponents;

	for (unsigned int i = 0; i < paths.size(); i++) {
		unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrComponents, 0);
		if (data) {
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Skybox textures failed to load at path: " << paths[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	size_t lastSlashPos = name.find_last_of("/\\");
	if (lastSlashPos != std::string::npos) {
		// Extract the file name
		name = name.substr(lastSlashPos + 1);
	}
}

Texture::~Texture() {
	glDeleteTextures(1, &ID);
}