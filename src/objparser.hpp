#pragma once

#include <string>
#include <utility>
#include <vector>

struct Vertex {
	float x;
	float y;
	float z;
	float tx;
	float ty;
	float nx;
	float ny;
	float nz;
};

struct Object {
	std::string name;
	std::vector<Vertex> vertices;
};

struct ObjFile {
	std::vector<Object> objects;
};

ObjFile parse_obj_file(std::string name);
