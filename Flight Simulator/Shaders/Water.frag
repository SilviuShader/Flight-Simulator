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
uniform sampler2D RefractionDepthTexture;
uniform sampler2D ReflectionDepthTexture;
uniform sampler2D WaterTexture;
uniform sampler2D WaterNormalMap;

uniform float FadeWaterDepth;
			  
uniform float ReflectivePower;
uniform float TextureMultiplier;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform mediump vec3 CameraPosition;

uniform float SpecularStrength;

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
	vec2 clipTexCoords    = FSInputReflectionPosition.xy / FSInputReflectionPosition.w * 0.5 + 0.5;
	vec2 refractTexCoords = clipTexCoords;
	vec2 reflectTexCoords = vec2(clipTexCoords.x, 1.0 - clipTexCoords.y);

	float belowDepth   = linearizeDepth(texture(RefractionDepthTexture, refractTexCoords).x, Near, Far);
	float aboveDepth   = linearizeDepth(texture(ReflectionDepthTexture, refractTexCoords).x, Near, Far);

	belowDepth = min(belowDepth, aboveDepth);

	float currentDepth = linearizeDepth(gl_FragCoord.z, Near, Far);

	float depthDifference = belowDepth - currentDepth;

	float finalAlpha = clamp(depthDifference / FadeWaterDepth, 0, 1);

	vec2 displacedTexCoords = FSInputTexCoords;

	vec3 normalData       = texture(WaterNormalMap, displacedTexCoords).rgb * 2.0 - vec3(1.0, 1.0, 1.0);
	vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);
	FSOutFragColor = AmbientColor;
	
	vec4 refractionColor = texture(RefractionTexture, refractTexCoords);
	vec4 reflectionColor = texture(ReflectionTexture, reflectTexCoords);

	vec3 waterToCamera = normalize(FSInputWaterToCamera);
	float reflectiveness = dot(waterToCamera, normal);

	reflectiveness = pow(reflectiveness, ReflectivePower);

	vec4 albedo = mix(refractionColor, reflectionColor, 1.0 - reflectiveness);
	albedo = mix(albedo, texture(WaterTexture, displacedTexCoords), TextureMultiplier);

	vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

	if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition.xyz);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0, 1.0);
	FSOutFragColor = FSOutFragColor * albedo;

	FSOutFragColor += SpecularStrength * specular * DiffuseColor;
    FSOutFragColor = FSOutFragColor;
	FSOutFragColor = FSOutFragColor;
	FSOutFragColor.a = finalAlpha;
}