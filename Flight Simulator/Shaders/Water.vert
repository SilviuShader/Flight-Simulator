#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 CameraPosition;

out vec2 FSInputTexCoords;
out vec4 FSInputReflectionPosition;
out vec3 FSInputWaterToCamera;

void main()
{
	vec4 worldPosition        = Model * vec4(VSInputPosition, 1.0);
	FSInputReflectionPosition = Projection * View * worldPosition;
    gl_Position               = FSInputReflectionPosition;
	FSInputTexCoords          = VSInputTexCoords;
	FSInputWaterToCamera      = CameraPosition - worldPosition.xyz;
}