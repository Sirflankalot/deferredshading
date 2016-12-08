#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec3 vTexCoords;

out vec4 FragColor;

const vec3 sundir = vec3(1, 1, 0); // Sun Direction

void main() {
    float in_sun = clamp(dot(normalize(vNormal), normalize(sundir)) * 3.0, -1, 1) * 0.5 + 0.5;
	FragColor = vec4(vec3(1.0, 0.2176, 0.028991) * in_sun + vec3(0.027, 0.027, 0.027), 1.0);
}