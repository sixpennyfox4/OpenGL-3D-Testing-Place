#pragma once

#include "glad/glad.h"

class VBO
{
public:
	GLuint ID;
	VBO() : ID(0) {}
	VBO(GLfloat* vertices, GLsizeiptr size);

	void setup(GLfloat* vertices, GLsizeiptr size);
	void Bind();
	void Unbind();
	void Delete();
};