#version 430 core

in vec2 FSInputTexCoords;
in vec3 FSInputNormal;

uniform vec4 AmbientColor;
uniform vec4 DiffuseColor;
uniform vec3 LightDirection;

uniform sampler2D TerrainTexture;

out vec4 FSOutFragColor;

void main()
{
    vec4 textureColor = texture(TerrainTexture, FSInputTexCoords);

    FSOutFragColor = AmbientColor;

    vec3 lightDir = -LightDirection;
    float lightIntensity = clamp(dot(FSInputNormal, lightDir), 0.0, 1.0);

    if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0f, 1.0f);

    FSOutFragColor = FSOutFragColor * textureColor;
}