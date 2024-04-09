#version 430 core

layout (location = 0) in vec3 aPos;

// camera properties
layout (std140, binding = 0) uniform Camera {
	// projection and view matrices
	mat4 proj;
	mat4 view;

	// camera position
    vec3 viewPos;

	// screen size
	vec2 screenSize;
};

out vec3 texCoords;

void main() {
	texCoords = aPos;
	vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0f);
	gl_Position = pos.xyww;
}