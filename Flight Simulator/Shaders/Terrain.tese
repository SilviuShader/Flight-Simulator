#version 430 core

#define NOISE_SAMPLES_COUNT 256

layout (triangles, equal_spacing, ccw) in;

layout (std140, binding=2) uniform NoiseValues
{
    vec4 Samples[NOISE_SAMPLES_COUNT];
};

uniform mat4 View;
uniform mat4 Projection;

in vec3 TESInputPosition[];
in vec3 TESInputColor[];

out vec3 FSInputColor;

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

void main()
{
    vec3 worldPosition = interpolate3D(TESInputPosition[0], TESInputPosition[1], TESInputPosition[2]);
    FSInputColor = interpolate3D(TESInputColor[0], TESInputColor[1], TESInputColor[2]) * float(Samples[255].z);
    gl_Position = Projection * View * vec4(worldPosition, 1.0f);
}