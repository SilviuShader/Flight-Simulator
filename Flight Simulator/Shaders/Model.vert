#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec3 VSInputNormal;
layout (location = 2) in vec2 VSInputTexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 FSInputWorldPosition;
out vec3 FSInputNormal;
out vec2 FSInputTexCoords;

void main()
{
	FSInputWorldPosition = (Model * vec4(VSInputPosition, 1.0)).xyz;
	FSInputNormal        = mat3(transpose(inverse(Model))) * VSInputNormal;
	FSInputTexCoords     = VSInputTexCoords;

	gl_Position          = Projection * View * Model * vec4(VSInputPosition, 1.0);
}