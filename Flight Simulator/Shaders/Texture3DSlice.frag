#version 430 core

in vec2 FSInputTexCoords;

uniform sampler3D Image;
uniform float     Slice;

out vec4 FSOutFragColor;

void main()
{
	FSOutFragColor = texture(Image, vec3(FSInputTexCoords, Slice));
}