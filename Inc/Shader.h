#pragma once

#include "glad/glad.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string get_file_contents(const char* filename);

class Shader
{
public:
	GLuint ID;
	Shader(const char* vertexSource, const char* fragmentSource);

	void Activate();
	int GetUniformLoc(const char* name);
	void Delete();
};