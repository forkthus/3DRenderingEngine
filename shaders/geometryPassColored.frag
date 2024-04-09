#version 430 core
#define MAX_NUM_LIGHTS 128
#define MAX_NUM_TEXTURES 5

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

// material properties
layout(std140, binding = 2) uniform Material {
    // common
    float shininess;
	uint isColor;

    // color
    vec3 ambientMat;
	vec3 diffuseMat;
	vec3 specularMat;
	float padding;

    // number of textures of each type
    uint diffuseCount;
    uint specularCount;
    uint normalCount;
    uint heightCount;
};

uniform sampler2D texture_diffuse[MAX_NUM_TEXTURES];
uniform sampler2D texture_specular[MAX_NUM_TEXTURES];
uniform sampler2D texture_normal[MAX_NUM_TEXTURES];
uniform sampler2D texture_height[MAX_NUM_TEXTURES];

in vec3 Normal;
in vec3 fragPos;
in vec2 TextCoords;
in vec3 tangentViewPos;
in vec3 tangentFragPos;
in mat3 TBN;

uniform float heightScale;
uniform float minLayers;
uniform float maxLayers;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	vec2 curTexCoords = texCoords;
	float height = texture(texture_height[0], curTexCoords).r;
	// adjust the number of layers based on the angle of view
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0f, 0.0f, 1.0f), viewDir)));
	float deltaDepth = 1.0f / numLayers;
	float currentDepth = 0.0f;
	vec2 P = viewDir.xy / viewDir.z * heightScale;
	vec2 deltaP = P / numLayers;
	
	while (currentDepth < height) {
		curTexCoords -= deltaP;
		currentDepth += deltaDepth;
		height = texture(texture_height[0], curTexCoords).r;
	}

	// interpolate the two closest depth values to get a more accurate result
	vec2 prevTexCoords = curTexCoords + deltaP;
	float prevDepth = texture(texture_height[0], prevTexCoords).r - (currentDepth - deltaDepth);

	float afterDepth = height - currentDepth;

	float weight = afterDepth / (afterDepth - prevDepth);

	return prevTexCoords * weight + curTexCoords * (1.0f - weight);
}

void main() {
	vec2 texCoords = TextCoords;
	// save the fragment position into the first texture
    gPosition = fragPos;
	// save the normal into the second texture
	gNormal = Normal;
	// save the albedo(diffuse) and specular into the third texture
	vec3 diffuse = vec3(0.0f);
	float specular = 0.0f;

	diffuse = diffuseMat;
	specular = specularMat.r;

	gAlbedoSpec.rgb = diffuse;
	gAlbedoSpec.a = specular;
}