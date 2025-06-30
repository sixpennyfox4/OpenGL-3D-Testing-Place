#pragma once

#include "glad/glad.h"

class EBO
{
public:
	GLuint ID;
	EBO() : ID(0) {}
	EBO(GLuint* indices, GLsizeiptr size);

	void setup(GLuint* indices, GLsizeiptr size);
	void Bind();
	void Unbind();
	void Delete();
};