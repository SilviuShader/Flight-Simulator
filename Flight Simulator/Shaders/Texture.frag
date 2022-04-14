#version 430 core

in vec2 FSInputTexCoords;

uniform sampler2D Image;

out vec4 FSOutFragColor;

void main()
{
	FSOutFragColor = texture(Image, FSInputTexCoords);
}