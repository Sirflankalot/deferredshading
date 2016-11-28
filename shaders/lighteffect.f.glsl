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
	vec3 Normal  = normalize(texture(gNormal, texcoords).rgb);
	vec3 Diffuse = texture(gAlbedoSpec, texcoords).rgb;
	float Spec   = texture(gAlbedoSpec, texcoords).a;

	// Calculate lighting
    // Diffuse
    vec3 lightDir = normalize(vLPosition - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * vLColor * Diffuse;
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0);
    //vec3 reflectDir = reflect(-lightDir, Normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = vLColor * spec;
    // Attenuation
    float dist = length(vLPosition - FragPos);
    float attenuation = 1.0 / ((1.0) + (0.7 * dist) + (1.8 * dist * dist));
    diffuse *= attenuation;
    specular *= attenuation;

    FragColor = vec4(diffuse + specular, 1.0);

 	//FragColor = vec4(Diffuse, 1.0f);
}