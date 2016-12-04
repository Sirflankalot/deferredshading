#include "objparser.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <tuple>

#include "util.hpp"

ObjFile parse_obj_file(std::string name) {
	auto raw_file = file_contents(name.c_str());

	ObjFile file;

	size_t line = 1;

	auto error_impl = [&line](const char* val, size_t fileline) {
		std::stringstream print;
		print << line << ": .obj parsing failed on line " << fileline << ": " << val << '\n';
		std::cerr << print.str();
		throw std::runtime_error(print.str().c_str());
	};

#define error(v) error_impl(v, __LINE__)

	std::istringstream fs(raw_file);

	if (!fs) {
		std::cerr << "Couldn't open file " << name << '\n';
		throw std::runtime_error("File opening failed");
	}

	std::vector<std::tuple<float, float, float>> vertices;
	std::vector<std::tuple<float, float>> texcoords;
	std::vector<std::tuple<float, float, float>> normals;

	while (!fs.eof()) {
		// Start of line
		char c;
		c = fs.get();
		switch (c) {
			// Skip comment line by fallthrough

			// Object name
			case 'o': {
				std::string name;
				fs >> name;

				file.objects.push_back(Object{std::move(name), {}});
				break;
			}

			// Parse a vertex
			case 'v': {
				c = fs.get();
				switch (c) {
					case ' ': {
						float x, y, z;
						bool fail = false;

						fail |= !(fs >> x);
						fail |= !(fs >> y);
						fail |= !(fs >> z);

						if (fail) {
							error("Couldn't find vertex values");
						}

						vertices.emplace_back(x, y, z);
						break;
					}

					case 't': {
						float tx, ty;
						bool fail = false;

						fail |= !(fs >> tx);
						fail |= !(fs >> ty);

						if (fail) {
							error("Couldn't find texture values");
						}

						texcoords.emplace_back(tx, ty);
						break;
					}

					case 'n': {
						float nx, ny, nz;
						bool fail = false;

						fail |= !(fs >> nx);
						fail |= !(fs >> ny);
						fail |= !(fs >> nz);

						if (fail) {
							error("Couldn't find normal coordinates");
						}

						normals.emplace_back(nx, ny, nz);
						break;
					}

					default: {
						error("Unknown argument to v");
						break;
					}
				}
				break;
			}

			// Create a face
			case 'f': {
				// Get each set of vertexes
				for (size_t i = 0; i < 3; ++i) {
					size_t vindex, vtindex, vnindex;
					bool vfail, vtfail, vnfail;

					vfail = !(fs >> vindex);
					if (vfail) {
						error("Vertex index not found");
					}
					// skip /
					fs.ignore();

					vtfail = !(fs >> vtindex);
					fs.clear();
					fs.ignore();

					vnfail = !(fs >> vnindex);
					fs.clear();

					// Push a vertex
					file.objects.back().vertices.push_back(Vertex{
					    std::get<0>(vertices[vindex - 1]),                // x
					    std::get<1>(vertices[vindex - 1]),                // y
					    std::get<2>(vertices[vindex - 1]),                // z
					    vtfail ? 0 : std::get<0>(texcoords[vtindex - 1]), // texcoord x
					    vtfail ? 0 : std::get<1>(texcoords[vtindex - 1]), // texcoord y
					    vnfail ? 0 : std::get<0>(normals[vnindex - 1]),   // normal x
					    vnfail ? 0 : std::get<1>(normals[vnindex - 1]),   // normal y
					    vnfail ? 0 : std::get<2>(normals[vnindex - 1])    // normal z
					});
				}
				break;
			}
		}

		std::string _;
		std::getline(fs, _);
		line++;
	}

#undef error

	return file;
}
