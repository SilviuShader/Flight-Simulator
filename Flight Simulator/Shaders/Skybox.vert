#version 430 core

layout (location = 0) in vec3 VSInputPosition;

out vec3 FSInputTexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	FSInputTexCoords = VSInputPosition;
	gl_Position      = Projection * View * Model * vec4(VSInputPosition, 1.0);
}