#version 430 core
#define MODEL_MATERIALS_COUNT 1

in vec2 FSInputTexCoords;
in vec4 FSInputReflectionPosition;
in vec3 FSInputNormal;
in vec3 FSInputTangent;
in vec3 FSInputBinormal;
in vec4 FSInputWorldPosition;
in vec4 FSInputRealClipCoords;

uniform sampler2D RefractionTexture;
uniform sampler2D ReflectionTexture;
uniform sampler2D RefractionDepthTexture;
uniform sampler2D ReflectionDepthTexture;

uniform sampler2D WaterTextures[MODEL_MATERIALS_COUNT];
uniform sampler2D WaterNormalTextures[MODEL_MATERIALS_COUNT];
uniform sampler2D WaterSpecularTextures[MODEL_MATERIALS_COUNT];

uniform float FadeWaterDepth;
			  
uniform float ReflectivePower;
uniform float TextureMultiplier;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform mediump vec3 CameraPosition;

uniform float Near;
uniform float Far;

out vec4 FSOutFragColor;

float linearizeDepth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main()
{
	vec2 realClipCoords   = FSInputRealClipCoords.xy / FSInputRealClipCoords.w * 0.5 + 0.5;

	float belowDepth   = linearizeDepth(texture(RefractionDepthTexture, realClipCoords).x, Near, Far);
	float aboveDepth   = linearizeDepth(texture(ReflectionDepthTexture, realClipCoords).x, Near, Far);

	belowDepth = min(belowDepth, aboveDepth);
	float currentDepth = linearizeDepth(gl_FragCoord.z, Near, Far);

	float depthDifference = belowDepth - currentDepth;
	
	float disturbance = clamp(depthDifference / 5.0, 0.0f, 1.0);

	vec2 alteredClipCoords = FSInputReflectionPosition.xy / FSInputReflectionPosition.w * 0.5 + 0.5;

	vec2 clipTexCoords    = mix(realClipCoords, alteredClipCoords, disturbance);
	vec2 refractTexCoords = clipTexCoords;
	vec2 reflectTexCoords = vec2(clipTexCoords.x, 1.0 - clipTexCoords.y);

	float finalAlpha = clamp(depthDifference / FadeWaterDepth, 0, 1);
	
	vec2 displacedTexCoords = FSInputTexCoords;

	vec3 normalData       = texture(WaterNormalTextures[0], displacedTexCoords).rgb * 2.0 - vec3(1.0, 1.0, 1.0);
	vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);
	FSOutFragColor = AmbientColor;

	float specularStrength  = texture(WaterSpecularTextures[0], displacedTexCoords).r;
	
	vec4 refractionColor = texture(RefractionTexture, refractTexCoords);
	vec4 reflectionColor = texture(ReflectionTexture, reflectTexCoords);

	vec3 waterToCamera = normalize(FSInputWorldPosition.xyz - CameraPosition);
	float reflectiveness = dot(waterToCamera, normal);

	reflectiveness = pow(reflectiveness, ReflectivePower);

	vec4 albedo = mix(refractionColor, reflectionColor, clamp(1.0 - reflectiveness, 0.0, 1.0));
	albedo = mix(albedo, texture(WaterTextures[0], displacedTexCoords), TextureMultiplier);

	vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

	if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfwayDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0, 1.0);
	FSOutFragColor = FSOutFragColor * albedo;

	FSOutFragColor += specularStrength * specular * DiffuseColor;
    FSOutFragColor = FSOutFragColor;
	FSOutFragColor = FSOutFragColor;
	FSOutFragColor.a = finalAlpha;
}