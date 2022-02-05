#version 430 core
layout (location = 0) in vec3 VSInputPosition;
layout (location = 1) in vec2 VSInputTexCoords;

uniform vec2 BottomLeft;
uniform vec2 TopRight;
uniform mat4 Model;

out vec3 TCSInputWorldPosition;
out vec3 TCSInputPosition;
out vec2 TCSInputTexCoords;

void main()
{
    vec2 inputPosition    = BottomLeft + VSInputPosition.xz * (TopRight - BottomLeft);
    TCSInputWorldPosition = (Model * (vec4(inputPosition.x, VSInputPosition.y, inputPosition.y, 1.0))).xyz;
    TCSInputPosition      = vec3(inputPosition.x, VSInputPosition.y, inputPosition.y);
    TCSInputTexCoords     = VSInputTexCoords;
}