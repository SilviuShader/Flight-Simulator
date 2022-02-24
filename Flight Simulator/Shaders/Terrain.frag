#version 430 core

#define TERRAIN_MATERIALS_COUNT 6

in vec3 FSInputWorldPosition;
in vec2 FSInputTexCoords;
in vec2 FSInputBiomeData;

in vec3 FSInputNormal;
in vec3 FSInputBinormal;
in vec3 FSInputTangent;

uniform float Gamma;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform vec3 CameraPosition;

uniform int BiomesCount;
uniform int MaterialsPerBiome;

uniform sampler2D BiomeMaterialsTexture;

uniform sampler2D TerrainTextures[TERRAIN_MATERIALS_COUNT];
uniform sampler2D TerrainNormalTextures[TERRAIN_MATERIALS_COUNT];
uniform sampler2D TerrainSpecularTextures[TERRAIN_MATERIALS_COUNT];

out vec4 FSOutFragColor;

void sampleMaterial(int materialIndex, out vec4 texColor, out vec3 normal, out float specularStrength)
{
    texColor = texture(TerrainTextures[materialIndex], FSInputTexCoords);
    normal = texture(TerrainNormalTextures[materialIndex], FSInputTexCoords).rgb;
    specularStrength = texture(TerrainSpecularTextures[materialIndex], FSInputTexCoords).r;

    normal *= 2.0f;
    normal -= vec3(1.0, 1.0, 1.0);
}

void StepsGradient(int totalValues, float t, out int index, out float percentage)
{
    int intervalsCount = 1 + (totalValues - 1) * 2;
    float floatVal = t * intervalsCount;

    int intVal = int(floor(floatVal));
    if (intVal >= intervalsCount)
        intVal = intervalsCount - 1;

    if (intVal % 2 == 0)
    {
        index =  (intVal / 2);
        percentage = 0.0;
    }
    else
    {
        
        index = (intVal - 1) / 2;
        percentage = floatVal - intVal;
    }
}

void sampleMaterialCombined(out vec4 texColor, out vec3 normal, out float specularStrength)
{
    int biomeIndex;
    float biomePercentage;

    StepsGradient(BiomesCount, FSInputBiomeData.x, biomeIndex, biomePercentage);

    int materialOrderIndex;
    float materialOrderPercentage;
    
    StepsGradient(MaterialsPerBiome, FSInputBiomeData.y, materialOrderIndex, materialOrderPercentage);
        
    float materialsCount = TERRAIN_MATERIALS_COUNT - 1;
    vec2 inputBiomeData = vec2(float(biomeIndex)         / float(BiomesCount       - 1), 
                               float(materialOrderIndex) / float(MaterialsPerBiome - 1)) * 
                          vec2(1.0 - 1.0 / float(BiomesCount), 
                               1.0 - 1.0 / float(MaterialsPerBiome));

    int currentMaterial           = int(texture(BiomeMaterialsTexture, inputBiomeData).r * materialsCount);
    int nextBiomeMaterial         = int(texture(BiomeMaterialsTexture, inputBiomeData + vec2(1.0 / float(BiomesCount), 0.0)).r * materialsCount);
    int nextAltitudeMaterial      = int(texture(BiomeMaterialsTexture, inputBiomeData + vec2(0.0, 1.0 / float(MaterialsPerBiome))).r * materialsCount);
    int nextBiomeAltitudeMaterial = int(texture(BiomeMaterialsTexture, inputBiomeData + vec2(1.0 / float(BiomesCount), 1.0 / float(MaterialsPerBiome))).r * materialsCount);

    vec4  currentTexture,          nextBiomeTexture,          nextAltitudeTexture,          nextBiomeAltitudeTexture;
    vec3  currentNormal,           nextBiomeNormal,           nextAltitudeNormal,           nextBiomeAltitudeNormal;
    float currentSpecularStrength, nextBiomeSpecularStrength, nextAltitudeSpecularStrength, nextBiomeAltitudeSpecularStrength;

    sampleMaterial(currentMaterial,           currentTexture,           currentNormal,           currentSpecularStrength);
    sampleMaterial(nextBiomeMaterial,         nextBiomeTexture,         nextBiomeNormal,         nextBiomeSpecularStrength);
    sampleMaterial(nextAltitudeMaterial,      nextAltitudeTexture,      nextAltitudeNormal,      nextAltitudeSpecularStrength);
    sampleMaterial(nextBiomeAltitudeMaterial, nextBiomeAltitudeTexture, nextBiomeAltitudeNormal, nextBiomeAltitudeSpecularStrength);
    
    vec4  bottomTexture  = mix(currentTexture,          nextBiomeTexture,          biomePercentage);
    vec3  bottomNormal   = mix(currentNormal,           nextBiomeNormal,           biomePercentage);
    float bottomSpecular = mix(currentSpecularStrength, nextBiomeSpecularStrength, biomePercentage);

    vec4  topTexture  = mix(nextAltitudeTexture,          nextBiomeAltitudeTexture,          biomePercentage);
    vec3  topNormal   = mix(nextAltitudeNormal,           nextBiomeAltitudeNormal,           biomePercentage);
    float topSpecular = mix(nextAltitudeSpecularStrength, nextBiomeAltitudeSpecularStrength, biomePercentage);

    texColor         = mix(bottomTexture,  topTexture,  materialOrderPercentage) * Gamma;
    normal           = mix(bottomNormal,   topNormal,   materialOrderPercentage);
    specularStrength = mix(bottomSpecular, topSpecular, materialOrderPercentage);
}

void main()
{
    vec4 textureColor;

    vec3 normalData;
    float specularStrength;

    sampleMaterialCombined(textureColor, normalData, specularStrength);

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