#version 330 core

out vec4 FragColor;
in vec3 vLPosition; // Light Position
in vec3 vLColor;    // Light Color

uniform sampler2D gPosition;   // World space position
uniform sampler2D gNormal;     // World space normals
uniform sampler2D gAlbedoSpec; // Albedo in rgb spec in a

uniform vec3 viewPos; // Viewport position
uniform vec2 resolution; // Screen Resolution

void main() {
	vec2 texcoords = (gl_FragCoord.xy / resolution);

	// Get data from gbuffer
	vec3 FragPos = texture(gPosition, texcoords).rgb;
	vec3 Normal  = texture(gNormal, texcoords).rgb;
	vec3 Albedo  = texture(gAlbedoSpec, texcoords).rgb;
	float Spec   = texture(gAlbedoSpec, texcoords).a;

	// Calculate lighting
    FragColor = vec4(Albedo, 1.0f);
}