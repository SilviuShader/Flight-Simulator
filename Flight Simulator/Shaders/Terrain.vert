#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;
layout (location = 2) in vec3 VSInputNormal;

uniform mat4 Model;

out vec3 TCSInputWorldPosition;
out vec3 TCSInputPosition;
out vec2 TCSInputTexCoords;
out vec3 TCSInputNormal;

void main()
{
    TCSInputWorldPosition = (Model * (vec4(VSInputPosition, 1.0f))).xyz;
    TCSInputPosition      = VSInputPosition;
    TCSInputTexCoords     = VSInputTexCoords;
    TCSInputNormal        = VSInputNormal;
}