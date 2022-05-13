#version 430 core
#define MODEL_MATERIALS_COUNT 1

in vec3 FSInputWorldPosition;
in vec3 FSInputNormal;
in vec2 FSInputTexCoords;
in vec3 FSInputBinormal;
in vec3 FSInputTangent;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform vec3 CameraPosition;

uniform sampler2D DiffuseTextures[MODEL_MATERIALS_COUNT];
uniform sampler2D NormalTextures[MODEL_MATERIALS_COUNT];
uniform sampler2D SpecularTextures[MODEL_MATERIALS_COUNT];

out vec4 FSOutFragColor;

void main()
{
	vec4  textureColor      = texture(DiffuseTextures[0],  FSInputTexCoords);
    vec3  normalData        = texture(NormalTextures[0],   FSInputTexCoords).rgb * 2.0 - vec3(1.0, 1.0, 1.0);
    float specularStrength  = texture(SpecularTextures[0], FSInputTexCoords).r;

    vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);
    FSOutFragColor = AmbientColor;

    vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

    if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfwayDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0, 1.0);

    FSOutFragColor = FSOutFragColor * textureColor;

    FSOutFragColor += specularStrength * specular * DiffuseColor;
    FSOutFragColor = FSOutFragColor;
}