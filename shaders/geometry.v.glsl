#version 330 core
  
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normals;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vFragPos;
out vec3 vTexCoords;

void main() {
	vec4 worldloc = world * vec4(position, 1.0);
    gl_Position = projection * view * worldloc;
    vNormal = mat3(transpose(inverse(world))) * normals;
    vFragPos = vec3(worldloc);
    vTexCoords = vec3(0, 0, 0);
}