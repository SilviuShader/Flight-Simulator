#version 430 core

in vec3 FSInputWorldPosition;
in vec2 FSInputTexCoords;

uniform sampler2D DiffuseTexture;

out vec4 FSOutFragColor;

void main()
{
	vec4 textureColor   = texture(DiffuseTexture,  FSInputTexCoords);
         FSOutFragColor = textureColor;
}