#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec3 vTexCoords;

out vec4 FragColor;

uniform vec3 lightposition;
uniform vec3 lightcolor;

uniform float radius;

void main() {
    const vec3 viewPos = vec3(0, 0, 0);
	vec3 normal = normalize(vNormal);
	// Calculate lighting
    // Diffuse
    vec3 lightDir = normalize(lightposition - vFragPos);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightcolor * vec3(1.0, 0.2176, 0.028991);
    // Specular
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    vec3 specular = spec * lightcolor;
    // Attenuation
    float dist = length(lightposition - vFragPos);
    // float attenuation = max(0, (1.0 / ((1.0) + (0.7 * dist) + (1.4 * dist * dist))) - (7.0 / 256.0));
    float attenuation = clamp(1.0 - dist/radius, 0.0, 1.0);
    attenuation *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    FragColor = vec4(diffuse + specular, 1.0);
}
