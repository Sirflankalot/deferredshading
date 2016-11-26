#include <GL/glew.h>

#include "shader.hpp"
#include "util.hpp"

#include <exception>
#include <iostream>
#include <sstream>

Shader_Program::Shader_Program() {
	this->program = glCreateProgram();
}

void Shader_Program::add(const char* filename, Shader::shadertype_t type) {
	GLenum new_type = 0;
	switch (type) {
		case Shader::VERTEX:
			new_type = GL_VERTEX_SHADER;
			break;
		case Shader::GEOMETRY:
			new_type = GL_GEOMETRY_SHADER;
			break;
		case Shader::TESS_C:
			new_type = GL_TESS_CONTROL_SHADER;
			break;
		case Shader::TESS_E:
			new_type = GL_TESS_EVALUATION_SHADER;
			break;
		case Shader::FRAGMENT:
			new_type = GL_FRAGMENT_SHADER;
			break;
		case Shader::COMPUTE:
			new_type = GL_COMPUTE_SHADER;
			break;
	}

	this->add(filename, new_type);
}

void Shader_Program::add(const char* filename, GLenum type) {
	std::string code = file_contents(filename);
	const char* code_ptr = code.c_str();

	GLuint ident;
	ident = glCreateShader(type);
	glShaderSource(ident, 1, &code_ptr, NULL);

	shaders.push_back(ident);
}

void Shader_Program::compile() {
	for (GLuint s : shaders) {
		glCompileShader(s);

		GLint success;
		glGetShaderiv(s, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[1024];
			glGetShaderInfoLog(s, sizeof(infoLog), NULL, infoLog);
			std::cerr << "Shader compilation failed:\n" << infoLog << '\n';
			throw std::runtime_error("Shader compilation failed.");
		}
	}
}

void Shader_Program::link() {
	for (GLuint s : shaders) {
		glAttachShader(this->program, s);
	}
	glLinkProgram(this->program);

	GLint success;
	glGetProgramiv(this->program, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetProgramInfoLog(this->program, sizeof(infoLog), NULL, infoLog);
		std::cerr << "Shader program linking failed:\n" << infoLog << '\n';
		throw std::runtime_error("Shader program linking failed.");
	}

	for (GLuint s : shaders) {
		glDeleteShader(s);
	}

	shaders.clear();
}

void Shader_Program::use() {
	glUseProgram(this->program);
}

GLuint Shader_Program::getUniform(const char* uniform_name, Shader::throwonfail_t should_throw) {
	GLint name = glGetUniformLocation(program, uniform_name);
	if (name == -1 && should_throw) {
		std::ostringstream err_str;
		err_str << "Can't find uniform named " << uniform_name << ".\n";
		std::cerr << err_str.str() << '\n';
		throw std::runtime_error(err_str.str().c_str());
	}

	return name;
}
