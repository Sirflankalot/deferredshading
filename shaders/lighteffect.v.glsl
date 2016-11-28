#version 330 core

layout (location = 7) in vec3 position;

uniform mat4 world;
uniform mat4 view;
uniform mat4 perspective;


void main() {
	gl_Position = perspective * view * world * vec4(position, 1.0f);
}
