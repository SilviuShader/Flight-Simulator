#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform mat4 ReflectionView;

out vec2 FSInputTexCoords;
out vec4 FSInputReflectionPosition;

void main()
{

    gl_Position               = Projection * View * Model * vec4(VSInputPosition, 1.0);
	FSInputReflectionPosition = Projection * View * Model * vec4(VSInputPosition, 1.0);
	FSInputTexCoords          = VSInputTexCoords;
}