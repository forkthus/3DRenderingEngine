#version 430 core

in vec4 fragPos;

uniform float far_plane;
uniform vec3 lightPos;

void main() {
    float lightDist = length(fragPos.xyz - lightPos);

    lightDist = lightDist / far_plane;

    gl_FragDepth = lightDist;
}