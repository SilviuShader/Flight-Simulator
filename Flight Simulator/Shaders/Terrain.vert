#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec3 VSInputColor;

out vec3 TCSInputPosition;
out vec3 TCSInputColor;

void main()
{
    TCSInputPosition    = VSInputPosition;
    TCSInputColor       = VSInputColor;
}