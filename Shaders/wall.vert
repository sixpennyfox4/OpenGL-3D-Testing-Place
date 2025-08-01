#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNormal;

out vec2 texCoord;

out vec3 Normal;
out vec3 crntPos;

uniform mat4 wallModel;
uniform mat4 camMatrix;
uniform vec2 texScale;

void main()
{
	crntPos = vec3(wallModel * vec4(aPos, 1.0f));

	gl_Position = camMatrix * vec4(crntPos, 1.0f);
	texCoord = aTex;
	Normal = aNormal;
}