#version 430 core
#define MAX_NUM_LIGHTS 128
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 gNormal[];
in vec3 gFragPos[];
in vec2 gTexCoords[]; 
in vec4 gFragPosLightSpace[][MAX_NUM_LIGHTS];

out vec3 Normal;
out vec3 fragPos;
out vec2 textureCoords;
out vec4 fragPosLightSpace[MAX_NUM_LIGHTS];

void main() {
	for(int i = 0; i < gl_in.length(); i++) {
        Normal = gNormal[i];
        fragPos = gFragPos[i];
        textureCoords = gTexCoords[i];
        gl_Position = gl_in[i].gl_Position;
        int baseIndex = i * MAX_NUM_LIGHTS;
        for(int j = 0; j < MAX_NUM_LIGHTS; j++) {
            fragPosLightSpace[j] = gFragPosLightSpace[i][j]; 
        }
        EmitVertex();
    }
    EndPrimitive();
}