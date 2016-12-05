#version 330 core

in vec3 vTexCoords;
in vec3 vNormal;
in vec3 vFragPos;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

const float NEAR = 0.5;
const float FAR = 1000;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
}

void main() {
	// Position vector
	gPosition.rgb = vFragPos;
	// Linear depth
	gPosition.a = LinearizeDepth(gl_FragCoord.z);
	// Normal vector
	gNormal = normalize(vNormal);
	// Diffuse color
	gAlbedoSpec.rgb = vec3(1.0, 0.2176, 0.028991);
	// Specular
	gAlbedoSpec.a = 1.0;
}
