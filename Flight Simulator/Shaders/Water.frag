#version 430 core

in vec2 FSInputTexCoords;
in vec4 FSInputReflectionPosition;

uniform sampler2D ReflectionTexture;

out vec4 FSOutFragColor;

void main()
{
	vec2 reflectTexCoords;

	reflectTexCoords.x =  FSInputReflectionPosition.x / FSInputReflectionPosition.w * 0.5 + 0.5;
	reflectTexCoords.y = -FSInputReflectionPosition.y / FSInputReflectionPosition.w * 0.5 + 0.5;

	FSOutFragColor = texture(ReflectionTexture, reflectTexCoords);
}