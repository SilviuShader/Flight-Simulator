#version 430 core
#define ALPHA_BIAS 0.2

in vec3 FSInputWorldPosition;
in vec2 FSInputTexCoords;

uniform sampler2D DiffuseTexture;

out vec4 FSOutFragColor;

void main()
{
	vec4 textureColor = texture(DiffuseTexture,  FSInputTexCoords);

	if(textureColor.a <= ALPHA_BIAS)
		discard;

    FSOutFragColor = textureColor;
	FSOutFragColor.a = 1.0;
}