#version 430 core
#define MAX_NUM_LIGHTS 128
#define MAX_NUM_TEXTURES 5

// structs definition
struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	mat4 lightSpaceMatrix;
};
// total: 4 * vec4 + 4 * vec4 = 8 * 16 = 128 bytes

struct PointLight {
    vec3 position;
    float padding;

    float constant;
    float linear;
    float quadratic;
	float far_plane;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	// mat4 lightSpaceMatrix;
};
// total: vec4 + vec4 + 3 * vec4 = 5 * 16 = 80 bytes

struct SpotLight {
    vec3 position;
    vec3 direction;
	float padding0;

    float cutOff;
    float outerCutOff;
	float padding1;
	float padding2;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       

    mat4 lightSpaceMatrix;
};
// total: vec4 + vec4 + vec4 + vec4 + 3 * vec4 + 4 * vec4 = 11 * 16 = 176 bytes

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

// lights properties
layout(std140, binding = 1) uniform Lights {
    // number of each type of lights
    uint dirLightCount;
    uint pointLightCount;
    uint spotLightCount;

    // vectors for lights
    DirLight dirLight[MAX_NUM_LIGHTS];
    PointLight pointLight[MAX_NUM_LIGHTS];
    SpotLight spotLight[MAX_NUM_LIGHTS];
};

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
uniform sampler2D shadowMap[MAX_NUM_TEXTURES];
uniform samplerCube shadowCubeMap[MAX_NUM_TEXTURES];

in vec3 Normal;
in vec3 fragPos;
in vec2 TextCoords;
in vec4 fragPosLightSpace[10];
in vec3 tangentViewPos;
in vec3 tangentFragPos;
in mat3 TBN;

out vec4 fragColor;

vec3 calcDirLight(DirLight light, uint index, vec3 norm, vec2 textureCoords);
vec3 calcPointLight(PointLight light, uint index, vec3 norm, vec2 textureCoords);
vec3 calcSpotLight(SpotLight light, uint index, vec3 norm, vec2 textureCoords);
float calcDirecShadow(uint index, float bias);
float calcPointShadow(uint index, float bias);

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
	vec3 norm = Normal;
	vec2 texCoords = TextCoords;
	if (isColor == 0) {
		if (heightCount > 0) {
			// get new texture coordinates
			vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
			texCoords = ParallaxMapping(texCoords, viewDir);
			if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
				discard;
		}
		// calculate norm based on normal maps
		vec3 normalTemp = vec3(0.0f);
		if (normalCount > 0) {
			for (uint i = 0; i < normalCount; i++) {
				normalTemp += texture(texture_normal[i], texCoords).rgb * 2.0 - 1.0;
			}
			norm = normalize(normalTemp);
		} 
	}
	// get Directional light
	vec3 dir = vec3(0.0f);
	for(uint i = 0; i < dirLightCount; i++) {
		dir += calcDirLight(dirLight[i], i, norm, texCoords);	
	}
	
	// get Point light
	vec3 point = vec3(0.0f);
	for(uint i = 0; i < pointLightCount; i++) {
		point += calcPointLight(pointLight[i], i, norm, texCoords);
	}

	// get Spot light
	vec3 spot = vec3(0.0f);
	for(uint i = 0; i < spotLightCount; i++) {
		spot += calcSpotLight(spotLight[i], i, norm, texCoords);
	}
	
	float a = diffuseCount > 0 ? texture(texture_diffuse[0], TextCoords).a : 1.0f;
	
	fragColor = vec4(dir + point + spot, a);
//	float gamma = 2.2;
//	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / gamma));
}

vec3 calcDirLight(DirLight light, uint index, vec3 norm, vec2 textureCoords) {
	//vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
	vec3 lightDir = (normalCount == 0) ? normalize(-light.direction) : normalize(TBN * (-light.direction));
	// vec3 norm = normalize(Normal);

	// ambient light
	vec3 ambient;

	// diffuse light
	float diffuseVar = max(dot(lightDir, norm), 0.0);
	vec3 diffuse;

	// specular light
	vec3 viewDir = (normalCount == 0) ? normalize(viewPos - fragPos) : normalize(tangentViewPos - tangentFragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	vec3 halfwayDir = normalize(lightDir + viewDir);
	// phong
	// float specularVar = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	// bling-phong
	float specularVar = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular;

	if (isColor == 1) {
		ambient = light.ambient * ambientMat;
		diffuse = light.diffuse * diffuseVar * diffuseMat;
		specular = light.specular * specularVar * specularMat;
	} else {
		for (uint i = 0; i < diffuseCount; i++) {
			ambient += texture(texture_diffuse[i], textureCoords).rgb * light.ambient;
			diffuse += texture(texture_diffuse[i], textureCoords).rgb * light.diffuse * diffuseVar;
		}

		for (uint i = 0; i < specularCount; i++) {
			specular += texture(texture_specular[i], textureCoords).rgb * light.specular * specularVar;
		}
	}

	float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
	float shadow = calcDirecShadow(index, bias);

	return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 calcPointLight(PointLight light, uint index, vec3 norm, vec2 textureCoords) {
	vec3 lightDir = (normalCount == 0) ? normalize(light.position - fragPos) : normalize(TBN * light.position - tangentFragPos);
	float dist = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + pow(light.linear * dist, 2.2) + pow(light.quadratic * dist * dist, 2.2));
	// vec3 norm = normalize(Normal);
	//float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	// ambient light
	vec3 ambient;
	
	// diffuse light
	float diffuseVar = max(dot(lightDir, norm), 0.0);
	vec3 diffuse;

	// specular light
	vec3 viewDir = (normalCount == 0) ? normalize(viewPos - fragPos) : normalize(tangentViewPos - tangentFragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	vec3 halfwayDir = normalize(lightDir + viewDir);
	// float specularVar = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	float specularVar = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular;

	if (isColor == 1) {
		ambient = light.ambient * ambientMat;
		diffuse = light.diffuse * diffuseVar * diffuseMat;
		specular = light.specular * specularVar * specularMat;
	} else {
		for (uint i = 0; i < diffuseCount; i++) {
			ambient += texture(texture_diffuse[i], textureCoords).rgb * light.ambient;
			diffuse += texture(texture_diffuse[i], textureCoords).rgb * light.diffuse * diffuseVar;
		}

		for (uint i = 0; i < specularCount; i++) {
			specular += texture(texture_specular[i], textureCoords).rgb * light.specular * specularVar;
		}
	}

	float shadow = calcPointShadow(index, 0.005);

	return (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;
}

vec3 calcSpotLight(SpotLight light, uint index, vec3 norm, vec2 textureCoords) {
	// Spot light
	vec3 lightDir = (normalCount == 0) ? normalize(light.position - fragPos) : normalize(TBN * light.position - tangentFragPos);
	float theta = dot(-lightDir, light.direction);
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// vec3 norm = normalize(Normal);

	// ambient light 
	vec3 ambient;

	// diffuse light
	float diffuseVar = max(dot(lightDir, norm), 0.0);
	vec3 diffuse;

	// specular light
	vec3 viewDir = (normalCount == 0) ? normalize(viewPos - fragPos) : normalize(tangentViewPos - tangentFragPos);
	vec3 reflectDir = normalize(reflect(-lightDir, norm));
	vec3 halfwayDir = normalize(lightDir + viewDir);
	// float specularVar = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	float specularVar = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular;

	if (isColor == 1) {
		ambient = light.ambient * ambientMat;
		diffuse = light.diffuse * diffuseVar * diffuseMat;
		specular = light.specular * specularVar * specularMat;
	} else {
		for (uint i = 0; i < diffuseCount; i++) {
			ambient += texture(texture_diffuse[i], textureCoords).rgb * light.ambient;
			diffuse += texture(texture_diffuse[i], textureCoords).rgb * light.diffuse * diffuseVar;
		}

		for (uint i = 0; i < specularCount; i++) {
			specular += texture(texture_specular[i], textureCoords).rgb * light.specular * specularVar;
		}
	}

	float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
	float shadow = calcDirecShadow(MAX_NUM_TEXTURES / 2 + index, bias);

	return (ambient + (1 - shadow) * (diffuse + specular)) * intensity;
}

float calcDirecShadow(uint index, float bias) {
	vec3 projCoords = fragPosLightSpace[index].xyz / fragPosLightSpace[index].w;
	projCoords = projCoords * 0.5 + 0.5;

	float currentDepth = projCoords.z;
	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(shadowMap[index], 0);

	// multisapling the shadow map for softener shadow
	for (int s = -1; s <= 1; s++) {
		for(int t = -1; t <= 1; t++) {
			float pcfDepth = texture(shadowMap[index], projCoords.xy + vec2(s, t) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}

float calcPointShadow(uint index, float bias) {
	vec3 sampleOffsetDirections[20] = vec3[](
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);   

	float shadow = 0;
	int samples = 20;
	float offset = 0.1;

	vec3 fragToLight = fragPos - pointLight[index].position;
	float currentDepth = length(fragToLight);
	float diskRadius = 0.05;
	float far_plane = pointLight[index].far_plane;

	for(int i = 0; i < samples; ++i) {
		float closestDepth = texture(shadowCubeMap[index], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;   
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}

	shadow /= float(samples); 

	return shadow;
}
		