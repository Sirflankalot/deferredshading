#version 330 core

in vec2 vTexCoords;

out float fragColor;

uniform sampler2D ssaoInput;

const int blursize = 3;

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
	float result = 0.0;
	for (int x = -blursize; x < blursize; ++x) {
		for (int y = -blursize; y < blursize; ++y) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoInput, vTexCoords + offset).r;
		}
	}

	fragColor = result / (blursize * blursize * 4);
	// fragColor = texture(ssaoInput, vTexCoords).r;
}