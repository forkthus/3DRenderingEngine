#version 430 core

out vec4 fragColor;

in vec2 TextCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 texel_offset = 1.0 / textureSize(image, 0);		// get the size of each step
	vec3 result = texture(image, TextCoords).rgb * weight[0]; 

	if (horizontal) {
		for (int i = 1; i < 5; i++) {
			result += texture(image, TextCoords + vec2(texel_offset.x * i, 0.0f)).rgb * weight[i];
			result += texture(image, TextCoords - vec2(texel_offset.x * i, 0.0f)).rgb * weight[i];
		}
	}
	else {
		for (int i = 1; i < 5; i++) {
			result += texture(image, TextCoords + vec2(0.0f, texel_offset.y * i)).rgb * weight[i];
			result += texture(image, TextCoords - vec2(0.0f, texel_offset.y * i)).rgb * weight[i];
		}
	}
	
	fragColor = vec4(result, 1.0);
}