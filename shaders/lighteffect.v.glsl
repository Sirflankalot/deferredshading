#version 330 core

layout (location = 1) in vec3 color;
layout (location = 2) in mat4 world;
layout (location = 6) in vec3 lightposition;
layout (location = 7) in vec3 position;

uniform mat4 view;
uniform mat4 perspective;

out vec3 vLPosition;
out vec3 vLColor;

void main() {
	gl_Position = perspective * view * world * vec4(position, 1.0f);
	vLPosition = lightposition;
	vLColor = color;
}
