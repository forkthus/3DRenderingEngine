#version 430 core

// camera properties
layout (std140, binding = 0) uniform Camera {
	// projection and view matrices
	mat4 proj;
	mat4 view;

	// camera position
    vec3 viewPos;

	// screen size
	vec2 screenSize;

	// exposure and gamma
	float exposure;
	float gamma;
};

in vec2 TextCoords;

uniform sampler2D hdrTex;
uniform sampler2D bloomTex;

out vec4 fragColor;

void main() {
	vec3 hdrColor = texture(hdrTex, TextCoords).rgb + texture(bloomTex, TextCoords).rgb;

	// exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	// gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));
	fragColor = vec4(mapped, 1.0f);
}