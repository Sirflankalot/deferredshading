#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D inval;
uniform float exposure;

void main() {
	vec3 hdrColor = texture(inval, vTexCoords).rgb;

	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	mapped = pow(mapped, vec3(1.0 / 2.2));

	FragColor = vec4(mapped, 1.0);
}
