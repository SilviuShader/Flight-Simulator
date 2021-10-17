#version 430 core

in vec2 FSInputTexCoords;

uniform sampler2D TerrainTexture;

out vec4 FSOutFragColor;

void main()
{
    FSOutFragColor = texture(TerrainTexture, FSInputTexCoords);
}