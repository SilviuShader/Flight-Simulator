#version 430 core

layout (vertices = 3) out;

in vec3 TCSInputPosition[];
in vec3 TCSInputColor[];

out vec3 TESInputPosition[];
out vec3 TESInputColor[];

void main()
{
    TESInputPosition[gl_InvocationID] = TCSInputPosition[gl_InvocationID];
    TESInputColor[gl_InvocationID] = TCSInputColor[gl_InvocationID];

    gl_TessLevelOuter[0] = 3.0;
    gl_TessLevelOuter[1] = 3.0;
    gl_TessLevelOuter[2] = 3.0;
    gl_TessLevelInner[0] = 3.0;

}