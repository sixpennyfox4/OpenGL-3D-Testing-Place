#include "Shader.h"

std::string get_file_contents(const char* fileName)
{
	std::ifstream in(fileName, std::ios::binary);
	if (in) {
		std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

		return(contents);
	}

	throw(errno);
}

Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertexShader{ glCreateShader(GL_VERTEX_SHADER) };
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link the shaders into a shader program.
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
	glUseProgram(ID);
}

int Shader::GetUniformLoc(const char* name)
{
	return glGetUniformLocation(ID, name);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}