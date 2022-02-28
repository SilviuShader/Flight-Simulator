#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec3 VSInputNormal;
layout (location = 2) in vec2 VSInputTexCoords;
layout (location = 3) in vec3 VSInputBinormal;
layout (location = 4) in vec3 VSInputTangent;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 FSInputWorldPosition;
out vec3 FSInputNormal;
out vec2 FSInputTexCoords;
out vec3 FSInputBinormal;
out vec3 FSInputTangent;

void main()
{
	FSInputWorldPosition = (Model * vec4(VSInputPosition, 1.0)).xyz;
	FSInputNormal        = mat3(transpose(inverse(Model))) * VSInputNormal;
	FSInputTexCoords     = VSInputTexCoords;
	FSInputBinormal      = mat3(transpose(inverse(Model))) * VSInputBinormal;
	FSInputTangent       = mat3(transpose(inverse(Model))) * VSInputTangent;

	gl_Position          = Projection * View * Model * vec4(VSInputPosition, 1.0);
}