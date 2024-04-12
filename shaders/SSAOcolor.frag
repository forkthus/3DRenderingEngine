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

out float occlusionFactor;

uniform sampler2D noiseTexture;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform vec3 samples[64];
uniform int noiseSize;

// tile the noise texture over the screen
const vec2 noiseScale = vec2(screenSize.x / float(noiseSize), screenSize.y / float(noiseSize));
const float bias = 0.025;
const float radius = 0.5;

void main() {
	vec3 fragPos = vec3(view * vec4(texture(gPosition, TextCoords).rgb, 1.0f));
	vec3 normal = texture(gNormal, TextCoords).rgb;
	vec3 randomVec = normalize(texture(noiseTexture, TextCoords * noiseScale).rgb);

	// applying Gramm Schmidt orthogonalization to the random vector
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));		// make randomVec perpendicular to the normal by subtracting the projection of randomVec onto the normal from randomVec
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < 64; i++) {
		// convert the world-space position of the sample to view-space
		vec3 viewPos = fragPos + TBN * samples[i] * radius; 
		// convert the view-space position to clip-space
		vec4 screenPos = proj * vec4(viewPos, 1.0);
		// convert the clip-space position to screen-space by dividing by the w component (normalized device coordinates)
		screenPos.xyz /= screenPos.w;
		// clip it to (0, 1) as we need screen-space position to calculate screen space amibent occlusion
		screenPos.xyz = 0.5 * screenPos.xyz + 0.5;

		float sampleDepth = vec3(view * vec4(texture(gPosition, screenPos.xy).rgb, 1.0f)).z;
		// make the transition smoother
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= viewPos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusionFactor = 1.0 - (occlusion / 64.0);
}