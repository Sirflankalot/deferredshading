#version 330 core

in vec3 vColor;

out vec4 fragcolor;

void main() {
	fragcolor = vec4(vColor, 1.0f);
}