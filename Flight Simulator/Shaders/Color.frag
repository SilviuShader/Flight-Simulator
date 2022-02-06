#version 430 core
in vec3 FSInputColor;

out vec4 FSOutFragColor;

void main()
{
	FSOutFragColor = vec4(FSInputColor, 1.0);
}