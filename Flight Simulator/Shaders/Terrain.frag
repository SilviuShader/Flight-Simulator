#version 430 core

in vec2 FSInputTexCoords;
in vec3 FSInputNormal;

uniform vec4 DiffuseColor;
uniform vec3 LightDirection;

uniform sampler2D TerrainTexture;

out vec4 FSOutFragColor;

void main()
{
    vec4 textureColor = texture(TerrainTexture, FSInputTexCoords);
    vec3 lightDir = -LightDirection;
    float lightIntensity = clamp(dot(FSInputNormal, lightDir), 0.0, 1.0);

    FSOutFragColor = clamp(lightIntensity * DiffuseColor, 0.0, 1.0);
    FSOutFragColor = FSOutFragColor * textureColor;
}