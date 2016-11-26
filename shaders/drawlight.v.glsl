#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in mat4 world;

uniform mat4 view;
uniform mat4 perspective;

out vec3 vColor;

void main() {
	gl_Position = perspective * view * world * vec4(position, 1.0f);
	vColor = color;
}
