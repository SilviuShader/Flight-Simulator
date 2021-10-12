#version 330 core
layout (location = 0) in vec3 InputPosition;
layout (location = 1) in vec3 InputColor;

out vec3 OutputColor;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * View * Model * vec4(InputPosition, 1.0);
    OutputColor = InputColor;
}