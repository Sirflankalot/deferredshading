#version 330 core

out vec4 FragColor;

uniform sampler2D gPosition;   // World space position
uniform sampler2D gNormal;     // World space normals
uniform sampler2D gAlbedoSpec; // Albedo in rgb spec in a

// uniform vec3 viewPos; // Viewport position
uniform vec2 resolution; // Screen Resolution

uniform vec3 lightposition;
uniform vec3 lightcolor;

uniform float radius;

void main() {
    const vec3 viewPos = vec3(0, 0, 0);

	vec2 texcoords = (gl_FragCoord.xy / resolution);

	// Get data from gbuffer
	vec3 FragPos = texture(gPosition, texcoords).rgb;
	vec3 Normal  = normalize(texture(gNormal, texcoords).rgb);
	vec3 Diffuse = texture(gAlbedoSpec, texcoords).rgb;
	float Spec   = texture(gAlbedoSpec, texcoords).a;

	// Calculate lighting
    // Diffuse
    vec3 lightDir = normalize(lightposition - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * lightcolor * Diffuse;
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = lightcolor * spec;
    // Attenuation
    float dist = length(lightposition - FragPos);
    float attenuation = clamp(1.0 - dist/(radius), 0.0, 1.0);
    attenuation *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    FragColor = vec4(diffuse + specular, 1.0);

 	//FragColor = vec4(Diffuse, 1.0f);
}