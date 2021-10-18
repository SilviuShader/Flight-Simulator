#version 430 core

layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputCoords;

out vec2 FSInputCoords;

void main()
{
    gl_Position   = vec4(VSInputPosition, 1.0);
    FSInputCoords = VSInputCoords;
}