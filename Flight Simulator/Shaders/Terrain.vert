#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec3 VSInputColor;

uniform mat4 Model;

out vec3 TCSInputPosition;
out vec3 TCSInputRawPosition;
out vec3 TCSInputColor;

void main()
{
    TCSInputPosition    = (Model * vec4(VSInputPosition, 1.0)).xyz;
    TCSInputRawPosition = VSInputPosition;
    TCSInputColor       = VSInputColor;
}