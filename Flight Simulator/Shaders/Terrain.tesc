#version 430 core

layout (vertices = 3) out;

in vec3 TCSInputWorldPosition[];
in vec3 TCSInputPosition[];
in vec3 TCSInputColor[];

uniform vec3 CameraPosition;

uniform float DistanceForDetails;
uniform float TessellationLevel;

out vec3 TESInputWorldPosition[];
out vec3 TESInputPosition[];
out vec3 TESInputColor[];

float getTessLevel(vec3 world, vec3 camera)
{
    float dist = distance(world, camera);
    float percentage = clamp(dist / DistanceForDetails, 0.0, 1.0);
    percentage = 1.0f - percentage;

    return mix(1.0f, TessellationLevel, percentage);
}

void main()
{
    TESInputWorldPosition[gl_InvocationID] = TCSInputWorldPosition[gl_InvocationID];
    TESInputPosition[gl_InvocationID]      = TCSInputPosition[gl_InvocationID];
    TESInputColor[gl_InvocationID]         = TCSInputColor[gl_InvocationID];

    gl_TessLevelOuter[0] = getTessLevel(TESInputWorldPosition[0], CameraPosition);
    gl_TessLevelOuter[1] = getTessLevel(TESInputWorldPosition[1], CameraPosition);
    gl_TessLevelOuter[2] = getTessLevel(TESInputWorldPosition[2], CameraPosition);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];

}