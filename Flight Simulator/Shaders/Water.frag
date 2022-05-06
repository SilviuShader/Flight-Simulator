#version 430 core

in vec2 FSInputTexCoords;
in vec4 FSInputReflectionPosition;

uniform sampler2D RefractionTexture;
uniform sampler2D ReflectionTexture;
uniform sampler2D DuTexture;
uniform sampler2D DvTexture;

uniform float     DisplacementStrength;

uniform float     MoveFactor;

out vec4 FSOutFragColor;

vec2 sampleDuDv(vec2 texCoords)
{
	vec2 result;
	result.x = texture(DuTexture, texCoords).x;
	result.y = texture(DvTexture, texCoords).x;

	result *= 2.0;
	result -= 1.0;

	return result;
}

void main()
{
	vec2 clipTexCoords = FSInputReflectionPosition.xy / FSInputReflectionPosition.w * 0.5 + 0.5;
	vec2 refractTexCoords = clipTexCoords;
	vec2 reflectTexCoords = vec2(clipTexCoords.x, 1.0 - clipTexCoords.y);

	vec2 displacedTexCoords = FSInputTexCoords;
	displacedTexCoords = FSInputTexCoords + sampleDuDv(displacedTexCoords + vec2(MoveFactor, 0.0)) * DisplacementStrength + vec2(0.0f, MoveFactor);

	vec2 displacement = sampleDuDv(displacedTexCoords) * DisplacementStrength;

	refractTexCoords += displacement;
	reflectTexCoords += displacement;

	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);
	
	vec4 refractionColor = texture(RefractionTexture, refractTexCoords);
	vec4 reflectionColor = texture(ReflectionTexture, reflectTexCoords);

	FSOutFragColor = mix(refractionColor, reflectionColor, 0.5);
}