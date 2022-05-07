#version 430 core

in vec2 FSInputTexCoords;
in vec4 FSInputReflectionPosition;
in vec3 FSInputWaterToCamera;
in vec3 FSInputNormal;
in vec3 FSInputTangent;
in vec3 FSInputBinormal;
in vec4 FSInputWorldPosition;

uniform sampler2D RefractionTexture;
uniform sampler2D ReflectionTexture;
uniform sampler2D DuTexture;
uniform sampler2D DvTexture;
uniform sampler2D RefractionDepthTexture;
uniform sampler2D WaterTexture;
uniform sampler2D WaterNormalMap;

uniform float     DisplacementStrength;

uniform float     MoveFactor;
uniform float     ReflectivePower;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform mediump vec3 CameraPosition;

uniform float Near;
uniform float Far;

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

float linearizeDepth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main()
{
	vec2 clipTexCoords    = FSInputReflectionPosition.xy / FSInputReflectionPosition.w * 0.5 + 0.5;
	vec2 refractTexCoords = clipTexCoords;
	vec2 reflectTexCoords = vec2(clipTexCoords.x, 1.0 - clipTexCoords.y);

	float belowDepth   = linearizeDepth(texture(RefractionDepthTexture, refractTexCoords).x, Near, Far);
	// TODO: Take reflection into account too (min distance), the displacement in the tessellation shader fucks up the math.
	float currentDepth = linearizeDepth(gl_FragCoord.z, Near, Far);

	float depthDifference = belowDepth - currentDepth;

	float displacementEdgeMultiplier = clamp(depthDifference / 10.0, 0, 1);
	float finalAlpha = clamp(depthDifference / 5.0, 0, 1);

	vec2 displacedTexCoords = FSInputTexCoords;
	displacedTexCoords = FSInputTexCoords + sampleDuDv(displacedTexCoords + vec2(MoveFactor, 0.0)) * DisplacementStrength + vec2(0.0, MoveFactor);

	vec3 normalData       = texture(WaterNormalMap, displacedTexCoords).rgb * 2.0 - vec3(1.0, 1.0, 1.0);
	vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);
	float specularStrength = 0.1;
	FSOutFragColor = AmbientColor;

	vec2 displacement = sampleDuDv(displacedTexCoords) * DisplacementStrength * displacementEdgeMultiplier;

	refractTexCoords += displacement;
	reflectTexCoords += displacement;

	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);
	
	vec4 refractionColor = texture(RefractionTexture, refractTexCoords);
	vec4 reflectionColor = texture(ReflectionTexture, reflectTexCoords);

	vec3 waterToCamera = normalize(FSInputWaterToCamera);
	float reflectiveness = dot(waterToCamera, normal);

	reflectiveness = pow(reflectiveness, ReflectivePower);

	vec4 albedo = mix(refractionColor, reflectionColor, 1.0 - reflectiveness);
	albedo = mix(albedo, texture(WaterTexture, displacedTexCoords), 0.05);

	vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

	if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition.xyz);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0, 1.0);
	FSOutFragColor = FSOutFragColor * albedo;

	FSOutFragColor += specularStrength * specular * DiffuseColor;
    FSOutFragColor = FSOutFragColor;
	FSOutFragColor = FSOutFragColor;
	FSOutFragColor.a = finalAlpha;
}