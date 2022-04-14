#version 430 core

in vec2 FSInputTexCoords;

uniform sampler2D SceneTexture;
uniform sampler2D DepthTexture;

out vec4 FSOutFragColor;

void main()
{
	vec3 sceneColor         = texture(SceneTexture, FSInputTexCoords).xyz;
	vec3 invertedSceneColor = 1.0 - sceneColor;

	vec4 depthColor = texture(DepthTexture, FSInputTexCoords);

	FSOutFragColor = vec4(sceneColor, 1.0) + depthColor * 0.001;
}