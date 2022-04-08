#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;

uniform float Scale;

out vec2 FSInputTexCoords;

void main()
{
	vec3 newPosition       = VSInputPosition * Scale;
	     newPosition      -= vec3(1.0, 1.0, 0.0) * (1.0 - Scale);
	     gl_Position       = vec4(newPosition, 1.0);
	     FSInputTexCoords  = VSInputTexCoords;
}