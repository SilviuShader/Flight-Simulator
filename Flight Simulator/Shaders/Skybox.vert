#version 430 core

layout (location = 0) in vec3 VSInputPosition;

out vec3 FSInputTexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec4 ClipPlane;

void main()
{
	FSInputTexCoords   = VSInputPosition;
	vec4 worldPosition = Model * vec4(VSInputPosition, 1.0);

	gl_ClipDistance[0] = dot(ClipPlane, worldPosition);
	gl_Position        = Projection * View * worldPosition;
}