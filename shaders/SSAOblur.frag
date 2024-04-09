#version 430 core

in vec2 TextCoords;

out float occlusionFactor;

uniform sampler2D SSAO;

uniform int noiseSize;

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(SSAO, 0));
	float result = 0.0;
	for (int x = -2; x < 2; x++)
		for (int y = -2; y < 2; y++) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(SSAO, TextCoords + offset).r;
		}

	occlusionFactor = result / float(noiseSize * noiseSize);
}