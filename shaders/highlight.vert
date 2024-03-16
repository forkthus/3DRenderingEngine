#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// camera properties
layout (std140, binding = 0) uniform Camera {
	// projection and view matrices
	mat4 proj;
	mat4 view;

	// camera position
    vec3 viewPos;
};

uniform mat4 model;

void main() {
	gl_Position = proj * view * model * vec4(aPos + aNormal * 0.01, 1.0f);
}