#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec3 VSInputNormal;
layout (location = 2) in vec2 VSInputTexCoords;
layout (location = 3) in vec3 VSInputBinormal;
layout (location = 4) in vec3 VSInputTangent;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 ChunkCenter;

uniform float TerrainWidth;
uniform float GridWidth;
uniform float GridHeight;
uniform float TerrainAmplitude;

uniform sampler2D NoiseTexture;

out vec3 FSInputWorldPosition;
out vec3 FSInputNormal;
out vec2 FSInputTexCoords;
out vec3 FSInputBinormal;
out vec3 FSInputTangent;

vec2 getUv(vec2 pos)
{
    pos = (pos / TerrainWidth);
    pos = vec2(pos.x * GridWidth, pos.y * GridHeight);
    vec2 uv = vec2(pos.x + (GridWidth / 2.0), pos.y + (GridHeight / 2.0));
    uv = vec2(uv.x / GridWidth, uv.y / GridHeight);
    
    return uv;
}

vec3 get3Dcoord(vec2 pos)
{
    vec2 uv = getUv(pos);

    float h = texture(NoiseTexture, uv).x;
    return vec3(pos.x, h * TerrainAmplitude, pos.y);
}

void main()
{
	FSInputWorldPosition    = (Model * vec4(VSInputPosition, 1.0)).xyz;

	vec3 positionInChunk    = FSInputWorldPosition - ChunkCenter;
	FSInputWorldPosition.y += get3Dcoord(positionInChunk.xz).y;

	FSInputNormal           = mat3(transpose(inverse(Model))) * VSInputNormal;
	FSInputTexCoords        = VSInputTexCoords;
	FSInputBinormal         = mat3(transpose(inverse(Model))) * VSInputBinormal;
	FSInputTangent          = mat3(transpose(inverse(Model))) * VSInputTangent;
                            
	gl_Position             = Projection * View * vec4(FSInputWorldPosition, 1.0);
}