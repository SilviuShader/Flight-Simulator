#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;

uniform mat4 Model;
uniform mediump mat4 View;
uniform mediump mat4 Projection;

uniform mediump vec3 CameraPosition;

out vec2 TCSInputTexCoords;
out vec4 TCSInputWorldPosition;
out vec4 TCSInputReflectionPosition;
out vec3 TCSInputWaterToCamera;

// TODO: Uniforms for all the hard-coded constants.

void main()
{
	TCSInputWorldPosition      = Model * vec4(VSInputPosition, 1.0);
	TCSInputReflectionPosition = Projection * View * TCSInputWorldPosition;
	TCSInputTexCoords          = VSInputTexCoords * 50.0;
	TCSInputWaterToCamera      = CameraPosition - TCSInputWorldPosition.xyz;
}