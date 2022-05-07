#version 430 core

layout (triangles, fractional_odd_spacing, ccw) in;

in vec2 TESInputTexCoords[];
in vec4 TESInputWorldPosition[];
in vec4 TESInputReflectionPosition[];
in vec3 TESInputWaterToCamera[];

uniform mediump mat4 View;
uniform mediump mat4 Projection;

uniform float Time;

out vec2 GSInputTexCoords;
out vec4 GSInputWorldPosition;
out vec4 GSInputReflectionPosition;
out vec3 GSInputWaterToCamera;

vec2 interpolate2D(vec2 u, vec2 v, vec2 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec4 interpolate4D(vec4 u, vec4 v, vec4 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 trochoidalWave(vec2 rotateDirection, vec2 xz, float speed, float offset, float radius)
{
    float spaceProjection = dot(rotateDirection, xz);
    vec3 result;

    float horizontalMovement = sin(Time * speed + spaceProjection * offset) * radius;
    result.x = horizontalMovement * xz.x;
    result.z = horizontalMovement * xz.y;

    result.y = cos(Time * speed + spaceProjection * offset) * radius * 30.0;

    return result;
}

vec3 vertexDisplacement(vec2 xz)
{
    // TODO: Combine more waves at different frequencies
    return trochoidalWave(vec2(1.0, 0.0), xz, 1.0, 1.0, 0.005);
}

void main()
{
    GSInputWorldPosition      = interpolate4D(TESInputWorldPosition[0],  TESInputWorldPosition[1],  TESInputWorldPosition[2]);

    GSInputWorldPosition.xyz += vertexDisplacement(GSInputWorldPosition.xz);

    GSInputTexCoords          = interpolate2D(TESInputTexCoords[0],          TESInputTexCoords[1],          TESInputTexCoords[2]);
    GSInputReflectionPosition = interpolate4D(TESInputReflectionPosition[0], TESInputReflectionPosition[1], TESInputReflectionPosition[2]);
    GSInputWaterToCamera      = interpolate3D(TESInputWaterToCamera[0],      TESInputWaterToCamera[1],      TESInputWaterToCamera[2]);

    gl_Position = Projection * View * GSInputWorldPosition;
}