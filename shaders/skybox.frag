#version 430 core

in vec3 texCoords;

out vec4 fragColor;

uniform samplerCube skybox;

void main() {
	fragColor = texture(skybox, texCoords);
//	float gamma = 2.2;
//	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / gamma));
}