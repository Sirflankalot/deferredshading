#pragma once

#include <GL/gl.h>
#include <vector>

namespace Shader {
	enum shadertype_t { VERTEX, GEOMETRY, TESS_C, TESS_E, FRAGMENT, COMPUTE };
	enum throwonfail_t { MANDITORY = 1, OPTIONAL = 0};
}

class Shader_Program {
  public:
	Shader_Program();

	void add(const char* filename, GLenum type);
	void add(const char* filename, Shader::shadertype_t type);
	void compile();
	void link();
	void use();

	GLuint getUniform(const char* uniform_name, Shader::throwonfail_t = Shader::OPTIONAL);
	GLuint getProgram() {
		return program;
	}

  private:
	std::vector<GLuint> shaders;
	GLuint program;

	void print_compile_errors(GLuint ident);
	void print_linker_errors(GLuint ident);
};
