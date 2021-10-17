#version 430 core

in vec2 FSInputTexCoords;
in vec3 FSInputNormal;

uniform sampler2D TerrainTexture;

out vec4 FSOutFragColor;

void main()
{
    FSOutFragColor = texture(TerrainTexture, FSInputTexCoords);
    FSOutFragColor = vec4(FSOutFragColor.xyz + FSInputNormal, 1.0);
}