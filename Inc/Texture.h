#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include "Shader.h"

class Texture
{
public:
	GLuint ID;
	GLenum type;
	GLuint unit;

	Texture(const char* image, GLenum texType, GLuint slot, GLenum interpolationType, GLenum texMappingType);

	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();
};