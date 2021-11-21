#version 430 core

in vec3 FSInputTexCoords;

uniform samplerCube Skybox;

out vec4 FSOutFragColor;

void main()
{
	FSOutFragColor = texture(Skybox, FSInputTexCoords);
}