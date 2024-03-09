#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>	
#include <fstream>
#include <sstream>
#include <iostream>

using std::string;

class Shader {
public:
	unsigned int ID;		// program ID
	string name;

	Shader(const char* vertPath, const char* fragPath, const char* geomPath = nullptr) {
		std::string vertString;
		std::string fragString;
		std::string geomString;

		std::ifstream vertFile;
		std::ifstream fragFile;
		std::ifstream geomFile;

		vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		geomFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			// open the files
			vertFile.open(vertPath);
			fragFile.open(fragPath);

			std::stringstream vertStream, fragStream, geomStream;

			vertStream << vertFile.rdbuf();
			fragStream << fragFile.rdbuf();

			vertFile.close();
			fragFile.close();

			vertString = vertStream.str();
			fragString = fragStream.str();

			if (geomPath) {
				geomFile.open(geomPath);
				geomStream << geomFile.rdbuf();
				geomFile.close();
				geomString = geomStream.str();
			}
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		const char* vertCode = vertString.c_str();
		const char* fragCode = fragString.c_str();
		const char* geomCode = geomString.c_str();

		// compile shaders
		unsigned int vert, frag, geom;

		// vertex shader
		vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert, 1, &vertCode, NULL);
		glCompileShader(vert);
		checkCompileErrors(vert, "VERTEX");

		// fragment shader
		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &fragCode, NULL);
		glCompileShader(frag);
		checkCompileErrors(frag, "FRAGMENT");

		// geometry shader
		if (geomPath) {
			geom = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geom, 1, &geomCode, NULL);
			glCompileShader(geom);
			checkCompileErrors(geom, "GEOMETRY");
		}

		// create shader program
		this->ID = glCreateProgram();
		name = "Shader " + std::to_string(this->ID);
		glAttachShader(this->ID, vert);
		glAttachShader(this->ID, frag);
		if (geomPath) {
			glAttachShader(this->ID, geom);
		}
		glLinkProgram(this->ID);
		checkCompileErrors(ID, "PROGRAM");

		// finish shader program initialization, delete
		glDeleteShader(vert);
		glDeleteShader(frag);
		if (geomPath)
			glDeleteShader(geom);
	}

	inline void use() {
		glUseProgram(this->ID);
	}
	inline void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
	}
	inline void setMat4(const std::string& name, glm::mat4& value) const {
		glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
	}
	inline void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
	}

private:
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
