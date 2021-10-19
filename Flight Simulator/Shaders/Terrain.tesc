#version 430 core

layout (vertices = 3) out;

in vec3 TCSInputWorldPosition[];
in vec3 TCSInputPosition[];
in vec2 TCSInputTexCoords[];

uniform vec3 CameraPosition;

uniform float DistanceForDetails;
uniform float TessellationLevel;

out vec3 TESInputWorldPosition[];
out vec3 TESInputPosition[];
out vec2 TESInputTexCoords[];

float getTessLevel(float dist1, float dist2)
{
    float dist = (dist1 + dist2) / 2;
    float percentage = clamp(dist / DistanceForDetails, 0.0, 1.0);
    percentage = 1.0 - percentage;

    return mix(1.0, TessellationLevel, percentage);
}

void main()
{
    TESInputWorldPosition[gl_InvocationID] = TCSInputWorldPosition[gl_InvocationID];
    TESInputPosition[gl_InvocationID]      = TCSInputPosition[gl_InvocationID];
    TESInputTexCoords[gl_InvocationID]     = TCSInputTexCoords[gl_InvocationID];

    float distVertex0 = distance(TESInputWorldPosition[0], CameraPosition);
    float distVertex1 = distance(TESInputWorldPosition[1], CameraPosition);
    float distVertex2 = distance(TESInputWorldPosition[2], CameraPosition);

    gl_TessLevelOuter[0] = getTessLevel(distVertex1, distVertex2);
    gl_TessLevelOuter[1] = getTessLevel(distVertex0, distVertex2);
    gl_TessLevelOuter[2] = getTessLevel(distVertex0, distVertex1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}