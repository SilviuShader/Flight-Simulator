#version 430 core

in vec2 FSInputTexCoords;
in vec4 FSInputReflectionPosition;

uniform sampler2D RefractionTexture;
uniform sampler2D ReflectionTexture;

out vec4 FSOutFragColor;

void main()
{
	vec2 texCoords = FSInputReflectionPosition.xy / FSInputReflectionPosition.w * 0.5 + 0.5;
	vec2 refractTexCoords = texCoords;
	vec2 reflectTexCoords = vec2(texCoords.x, 1.0 - texCoords.y);
	
	vec4 refractionColor = texture(RefractionTexture, refractTexCoords);
	vec4 reflectionColor = texture(ReflectionTexture, reflectTexCoords);

	FSOutFragColor = mix(refractionColor, reflectionColor, 0.2);
}