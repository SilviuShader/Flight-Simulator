#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 2) in vec2 VSInputTexCoords;

layout (location = 5) in mat4 VSInputModelMatrix;

uniform mat4 View;
uniform mat4 Projection;

uniform vec3 ChunkCenter;

uniform float TerrainWidth;
uniform float GridWidth;
uniform float GridHeight;
uniform float TerrainAmplitude;

uniform sampler2D NoiseTexture;

out vec3 FSInputWorldPosition;
out vec2 FSInputTexCoords;

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
    mat4 model                   = mat4(1.0);

	     FSInputWorldPosition    = (VSInputModelMatrix * vec4(VSInputPosition, 1.0)).xyz;

    vec3 worldOriginPosition     = (VSInputModelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	vec3 positionInChunk         = worldOriginPosition - ChunkCenter;
	     FSInputWorldPosition.y += get3Dcoord(positionInChunk.xz).y;

    mat3 normalWorldMat          = mat3(transpose(inverse(VSInputModelMatrix)));

	     FSInputTexCoords        = VSInputTexCoords;
                             
	     gl_Position             = Projection * View * vec4(FSInputWorldPosition, 1.0);
}