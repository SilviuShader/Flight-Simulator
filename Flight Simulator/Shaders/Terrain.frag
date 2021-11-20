#version 430 core

#define TERRAIN_MATERIALS_COUNT 2

in vec3 FSInputWorldPosition;
in vec2 FSInputTexCoords;

in vec3 FSInputNormal;
in vec3 FSInputBinormal;
in vec3 FSInputTangent;

uniform float Gamma;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform vec3  CameraPosition;

uniform sampler2D TerrainTextures[TERRAIN_MATERIALS_COUNT];
uniform sampler2D TerrainNormalTextures[TERRAIN_MATERIALS_COUNT];
uniform sampler2D TerrainSpecularTextures[TERRAIN_MATERIALS_COUNT];

out vec4 FSOutFragColor;

float getFirstTexturePercentage()
{
    return abs(dot(FSInputNormal, vec3(0.0f, 1.0f, 0.0f)));
}

void main()
{
    float firstPercentage = getFirstTexturePercentage();

    vec4 firstTexColor = texture(TerrainTextures[0], FSInputTexCoords) * firstPercentage;
    vec4 secondTexColor = texture(TerrainTextures[1], FSInputTexCoords) * (1.0 - firstPercentage);

    vec4 textureColor = clamp((firstTexColor + secondTexColor) * Gamma, 0.0, 1.0);

    vec3 normalData = ((texture(TerrainNormalTextures[0], FSInputTexCoords).xyz * firstPercentage) + (texture(TerrainNormalTextures[1], FSInputTexCoords).xyz * (1.0 - firstPercentage))) * 2.0;
    float specularStrength = (texture(TerrainSpecularTextures[0], FSInputTexCoords).x * firstPercentage) +  (texture(TerrainSpecularTextures[1], FSInputTexCoords).x * (1.0 - firstPercentage));

    normalData -= vec3(1.0f, 1.0f, 1.0f);

    vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);
    FSOutFragColor = AmbientColor;

    vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

    if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0f, 1.0f);

    FSOutFragColor = FSOutFragColor * textureColor;

    FSOutFragColor += specularStrength * specular * DiffuseColor;
}