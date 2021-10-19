#version 430 core

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

out vec2 FSInputTexCoords;
out vec3 FSInputNormal;

vec2 interpolate2D(vec2 u, vec2 v, vec2 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 get3Dcoord(vec2 pos)
{
    pos = (pos / TerrainWidth);
    pos = vec2(pos.x * GridWidth, pos.y * GridHeight);
    vec2 uv = vec2(pos.x + (GridWidth / 2.0), pos.y + (GridHeight / 2.0));
    uv = vec2(uv.x / GridWidth, uv.y / GridHeight);
    float h = texture(NoiseTexture, uv).x;
    return vec3(pos.x, h * TerrainAmplitude, pos.y);
}

vec3 calculateNormal(vec3 currentPos)
{
    float offset = 0.01;

    vec2 posx    = vec2(currentPos.x, currentPos.z) + vec2(offset, 0.0);
    vec2 posxneg = vec2(currentPos.x, currentPos.z) - vec2(offset, 0.0);
    vec2 posy    = vec2(currentPos.x, currentPos.z) + vec2(0.0, offset);
    vec2 posyneg = vec2(currentPos.x, currentPos.z) - vec2(0.0, offset);

    currentPos = get3Dcoord(currentPos.xz);
    vec3 right = get3Dcoord(posx);
    vec3 left = get3Dcoord(posxneg);
    vec3 top = get3Dcoord(posy);
    vec3 bottom = get3Dcoord(posyneg);
    
    vec3 normalTopRight = normalize(cross(top - currentPos, right - currentPos));
    vec3 normalBottomRight = normalize(cross(right - currentPos, bottom - currentPos));
    vec3 normalBottomLeft = normalize(cross(bottom - currentPos, left - currentPos));
    vec3 normalTopLeft = normalize(cross(left - currentPos, top - currentPos));

    //vec3 dx = get3Dcoord(posx, nposx) - get3Dcoord(posxneg, nposxneg);
    //vec3 dy = get3Dcoord(posy, nposy) - get3Dcoord(posyneg, nposyneg);

    // normalize(cross(dy, dx));

    vec3 result = normalize(normalTopRight + normalTopLeft + normalBottomLeft + normalBottomRight);
    return vec3(result.z, result.y, result.x);
}

void main()
{
    vec3 rawPosition   = interpolate3D(TESInputPosition[0], TESInputPosition[1], TESInputPosition[2]);
    vec3 worldPosition = interpolate3D(TESInputWorldPosition[0], TESInputWorldPosition[1], TESInputWorldPosition[2]);
    
    //rawPosition.y += noise.x * 50.0;
    worldPosition.y = get3Dcoord(rawPosition.xz).y;

    FSInputTexCoords   = interpolate2D(TESInputTexCoords[0], TESInputTexCoords[1], TESInputTexCoords[2]);
    FSInputNormal      = calculateNormal(rawPosition);
    gl_Position        = Projection * View * vec4(worldPosition, 1.0);
}