#include "util.hpp"

#include <fstream>
#include <iostream>
#include <string>

std::string file_contents(const char* filename) {
	std::ifstream f(filename);
	std::string str;

	if (!f.is_open()) {
		std::cerr << "Can't open " << filename << "\n";
		return std::string();
	}

	f.seekg(0, std::ios::end);
	str.reserve(f.tellg());
	f.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

	return str;
}