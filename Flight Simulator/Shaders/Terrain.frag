#version 430 core

in vec3 FSInputWorldPosition;
in vec2 FSInputTexCoords;
in vec4 FSInputColor;

in vec3 FSInputNormal;
in vec3 FSInputBinormal;
in vec3 FSInputTangent;

uniform vec4 AmbientColor;
uniform vec4 DiffuseColor;
uniform vec3 LightDirection;

uniform vec3  CameraPosition;
uniform float SpecularStrength;

uniform sampler2D TerrainTexture;
uniform sampler2D TerrainNormalTexture;

out vec4 FSOutFragColor;

void main()
{
    vec4 textureColor = clamp(texture(TerrainTexture, FSInputTexCoords) * FSInputColor * 2.0, 0.0, 1.0);
    vec3 normalData = texture(TerrainNormalTexture, FSInputTexCoords).xyz * 2.0;
    normalData -= vec3(1.0f, 1.0f, 1.0f);

    vec3 normal = (FSInputTangent * normalData.x) + (FSInputBinormal * normalData.y) + (FSInputNormal * normalData.z);

    FSOutFragColor = AmbientColor;

    vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

    if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0f, 1.0f);

    FSOutFragColor = FSOutFragColor * textureColor;

    FSOutFragColor += SpecularStrength * specular * DiffuseColor;
}