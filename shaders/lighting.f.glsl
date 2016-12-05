#version 330 core

out vec4 FragColor; // Output color
in vec2 vTexCoords; // Location on screen

uniform sampler2D gPosition;   // World space position
uniform sampler2D gNormal;     // World space normals
uniform sampler2D gAlbedoSpec; // Albedo in rgb spec in a
uniform sampler2D ssaoInput;

uniform vec3 viewPos; // Viewport position

const vec3 sundir = vec3(1, 1, 0); // Sun Direction

void main() {
	// Get data from gbuffer
	vec3 FragPos = texture(gPosition, vTexCoords).rgb;
	vec3 Normal  = texture(gNormal, vTexCoords).rgb;
	vec3 Albedo  = texture(gAlbedoSpec, vTexCoords).rgb;
	float Spec   = texture(gAlbedoSpec, vTexCoords).a;
	float ssao   = texture(ssaoInput, vTexCoords).r;

	// Ambient
	vec3 ambient = Albedo * vec3(0.027, 0.027, 0.027) * ssao;

	// Sun
    float in_sun = clamp(dot(Normal, normalize(sundir)) * 2.0, -1, 1) * 0.5 + 0.5;
    vec3 sun = Albedo * in_sun * ssao;

	// Calculate lighting
    FragColor = vec4(sun + ambient, 1.0);
}