#version 430 core

in vec3 FSInputTexCoords;

out vec4 FSOutFragColor;

void main()
{
	vec3 col = (FSInputTexCoords.xyz * 0.5) + vec3(0.5, 0.5, 0.5);
	FSOutFragColor = vec4(col.r, col.g, col.b, 1.0);
}