#version 400 core
out vec4 FragColor;

in vec3 OutputColor;

void main()
{
    FragColor = vec4(OutputColor, 1.0);
}