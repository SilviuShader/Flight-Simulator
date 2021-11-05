#version 430 core

#define TERRAIN_COLORS_COUNT 5

layout (triangles, fractional_odd_spacing, ccw) in;

layout (std140, binding = 0) uniform Colors
{
    vec4 TerrainColors[TERRAIN_COLORS_COUNT];
};

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
out vec4 FSInputColor;
out vec3 FSInputNormal;

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

vec3 calculateNormal(vec3 currentPos)
{
    float offset = 1.0 / 1024.0;

    vec2 posx    = currentPos.xz + vec2(offset, 0.0);
    vec2 posxneg = currentPos.xz - vec2(offset, 0.0);
    vec2 posy    = currentPos.xz + vec2(0.0, offset);
    vec2 posyneg = currentPos.xz - vec2(0.0, offset);

    currentPos = get3Dcoord(currentPos.xz);
    vec3 right = get3Dcoord(posx);
    vec3 left = get3Dcoord(posxneg);
    vec3 top = get3Dcoord(posy);
    vec3 bottom = get3Dcoord(posyneg);
    
    vec3 normalTopRight = normalize(cross(top - currentPos, right - currentPos));
    vec3 normalBottomRight = normalize(cross(right - currentPos, bottom - currentPos));
    vec3 normalBottomLeft = normalize(cross(bottom - currentPos, left - currentPos));
    vec3 normalTopLeft = normalize(cross(left - currentPos, top - currentPos));

    return normalize(normalTopRight + normalTopLeft + normalBottomLeft + normalBottomRight);
}

vec4 getColor(vec2 pos, float height)
{
    vec2 uv = getUv(pos);

    float texColor = texture(NoiseTexture, uv).y;

    float percentage = height / (TerrainAmplitude * 2.0) + mix(0.9, 3.0, texColor - 1.0);
    percentage *= 0.5;
    percentage = clamp(percentage, 0.0, 1.0);
    float fIndex = percentage * (TERRAIN_COLORS_COUNT - 1);
    int index = int(fIndex);
    if (index >= (TERRAIN_COLORS_COUNT - 1))
        return TerrainColors[(TERRAIN_COLORS_COUNT - 1)];

    return mix(TerrainColors[index], TerrainColors[index + 1], fIndex - index);
}

void main()
{
    vec3 rawPosition     = interpolate3D(TESInputPosition[0], TESInputPosition[1], TESInputPosition[2]);
    vec3 worldPosition   = interpolate3D(TESInputWorldPosition[0], TESInputWorldPosition[1], TESInputWorldPosition[2]);
    
    worldPosition.y      = get3Dcoord(rawPosition.xz).y;

    FSInputColor         = getColor(rawPosition.xz, worldPosition.y);

    FSInputWorldPosition = worldPosition;
    FSInputTexCoords     = interpolate2D(TESInputTexCoords[0], TESInputTexCoords[1], TESInputTexCoords[2]);
    FSInputNormal        = calculateNormal(rawPosition);
    gl_Position          = Projection * View * vec4(worldPosition, 1.0);
}