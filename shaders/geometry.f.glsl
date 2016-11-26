#version 330 core

in vec3 vTexCoords;
in vec3 vNormal;
in vec3 vFragPos;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

void main() {
	// Position vector
	gPosition = vFragPos;
	// Normal vector
	gNormal = normalize(vNormal);
	// Diffuse color
	gAlbedoSpec.rgb = vec3(1.0f, 0.5f, 0.2f);
	// Specular
	gAlbedoSpec.a = 1.0f;
} 