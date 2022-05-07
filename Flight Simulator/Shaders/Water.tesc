#version 430 core

layout (vertices = 3) out;

in vec2 TCSInputTexCoords[];
in vec4 TCSInputWorldPosition[];
in vec4 TCSInputReflectionPosition[];
in vec3 TCSInputWaterToCamera[];

uniform mediump vec3 CameraPosition;

uniform float DistanceForDetails;
uniform float TessellationLevel;

out vec2 TESInputTexCoords[];
out vec4 TESInputWorldPosition[];
out vec4 TESInputReflectionPosition[];
out vec3 TESInputWaterToCamera[];

float getTessLevel(float dist1, float dist2)
{
    float dist = (dist1 + dist2) / 2;
    float percentage = clamp(dist / DistanceForDetails, 0.0, 1.0);
    percentage = 1.0 - percentage;

    return mix(1.0, TessellationLevel, percentage);
}

void main()
{
    TESInputTexCoords[gl_InvocationID]          = TCSInputTexCoords[gl_InvocationID];
    TESInputWorldPosition[gl_InvocationID]      = TCSInputWorldPosition[gl_InvocationID];
    TESInputReflectionPosition[gl_InvocationID] = TCSInputReflectionPosition[gl_InvocationID];
    TESInputWaterToCamera[gl_InvocationID]      = TCSInputWaterToCamera[gl_InvocationID];

    float distVertex0 = distance(TESInputWorldPosition[0].xyz, CameraPosition);
    float distVertex1 = distance(TESInputWorldPosition[1].xyz, CameraPosition);
    float distVertex2 = distance(TESInputWorldPosition[2].xyz, CameraPosition);

    gl_TessLevelOuter[0] = getTessLevel(distVertex1, distVertex2);
    gl_TessLevelOuter[1] = getTessLevel(distVertex0, distVertex2);
    gl_TessLevelOuter[2] = getTessLevel(distVertex0, distVertex1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}