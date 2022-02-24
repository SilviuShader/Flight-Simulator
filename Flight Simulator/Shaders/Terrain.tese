#version 430 core

#define TERRAIN_COLORS_COUNT 5

layout (triangles, fractional_odd_spacing, ccw) in;

uniform mat4 View;
uniform mat4 Projection;

uniform float TerrainWidth;
uniform float GridWidth;
uniform float GridHeight;
uniform float TerrainAmplitude;

uniform sampler2D NoiseTexture;

in vec3 TESInputWorldPosition[];
in vec3 TESInputPosition[];
in vec2 TESInputTexCoords[];

out vec3 FSInputWorldPosition;
out vec2 FSInputTexCoords;
out vec2 FSInputBiomeData;

out vec3 FSInputNormal;
out vec3 FSInputBinormal;
out vec3 FSInputTangent;

vec2 interpolate2D(vec2 u, vec2 v, vec2 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

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

void calculateNormal(vec3 currentPos, out vec3 normal, out vec3 binormal, out vec3 tangent)
{
    float offset     = 1.0 / 32.0;

    vec2  posx       = currentPos.xz + vec2(offset, 0.0);
    vec2  posxneg    = currentPos.xz - vec2(offset, 0.0);
    vec2  posy       = currentPos.xz + vec2(0.0,    offset);
    vec2  posyneg    = currentPos.xz - vec2(0.0,    offset);

          currentPos = get3Dcoord(currentPos.xz);
    vec3  right      = get3Dcoord(posx);
    vec3  left       = get3Dcoord(posxneg);
    vec3  top        = get3Dcoord(posy);
    vec3  bottom     = get3Dcoord(posyneg);
                    
          tangent    = normalize(right - left);
          binormal   = normalize(top - bottom);
          normal     = normalize(cross(binormal, tangent));
}

vec2 calculateBiome(vec2 pos, float height)
{
    vec2 uv = getUv(pos);

    float texColor = texture(NoiseTexture, uv).y;

    return vec2(texColor, (height / TerrainAmplitude) * 2.0);
}

void main()
{
    vec3 rawPosition     = interpolate3D(TESInputPosition[0],      TESInputPosition[1],      TESInputPosition[2]);
    vec3 worldPosition   = interpolate3D(TESInputWorldPosition[0], TESInputWorldPosition[1], TESInputWorldPosition[2]);
    
    worldPosition.y      = get3Dcoord(rawPosition.xz).y;

    FSInputBiomeData     = calculateBiome(rawPosition.xz, worldPosition.y);
    FSInputWorldPosition = worldPosition;
    FSInputTexCoords     = interpolate2D(TESInputTexCoords[0], TESInputTexCoords[1], TESInputTexCoords[2]);
    gl_Position          = Projection * View * vec4(worldPosition, 1.0);

    calculateNormal(rawPosition, FSInputNormal, FSInputBinormal, FSInputTangent);
}