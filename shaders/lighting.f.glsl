#version 330 core

out vec4 FragColor; // Output color
in vec2 vTexCoords; // Location on screen

uniform sampler2D gPosition;   // World space position
uniform sampler2D gNormal;     // World space normals
uniform sampler2D gAlbedoSpec; // Albedo in rgb spec in a

uniform vec3 viewPos; // Viewport position

const vec3 sundir = vec3(1, 1, 0); // Sun Direction

void main() {
	// Get data from gbuffer
	vec3 FragPos = texture(gPosition, vTexCoords).rgb;
	vec3 Normal  = texture(gNormal, vTexCoords).rgb;
	vec3 Albedo  = texture(gAlbedoSpec, vTexCoords).rgb;
	float Spec   = texture(gAlbedoSpec, vTexCoords).a;

	// Calculate lighting
    float in_sun = clamp(dot(Normal, sundir) * 2.0, -1, 1) * 0.5 + 0.5;
    //FragColor = vec4(Albedo * in_sun + vec3(0, 0, 0.0063), 1.0);
    FragColor = vec4(vec3(0.0), 1.0);
}