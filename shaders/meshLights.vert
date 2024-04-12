#version 420 core
#define MAX_NUM_LIGHTS 128
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

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
// total: vec4 + vec4 + 3 * vec4  = 5 * 16 = 80 bytes

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

uniform mat4 model;

out vec3 Normal;
out vec3 fragPos;
out vec2 TextCoords;
out vec4 fragPosLightSpace[10];
out vec3 tangentViewPos;
out vec3 tangentFragPos;
out mat3 TBN;

void main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
	fragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TextCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 B = normalize(normalMatrix * aBitangent);
    
    TBN = transpose(mat3(T, B, N));

    tangentViewPos = TBN * viewPos;
    tangentFragPos = TBN * fragPos;

    for (int i = 0; i < dirLightCount && i < 5; i++)
	{
		fragPosLightSpace[i] = dirLight[i].lightSpaceMatrix * vec4(fragPos, 1.0);
	}
    for (int i = 0; i < spotLightCount && i < 5; i++)
    {
        fragPosLightSpace[i + 5] = spotLight[i].lightSpaceMatrix * vec4(fragPos, 1.0);
	}
}