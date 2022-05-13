#version 430 core
#define ALPHA_BIAS 0.2

in vec3 FSInputWorldPosition;
in vec3 FSInputNormal;
in vec2 FSInputTexCoords;

uniform sampler2D DiffuseTexture;

uniform vec4  AmbientColor;
uniform vec4  DiffuseColor;
uniform vec3  LightDirection;
uniform float SpecularPower; 

uniform vec3 CameraPosition;

out vec4 FSOutFragColor;

void main()
{
	vec4 textureColor = texture(DiffuseTexture,  FSInputTexCoords);
    vec3 normal = normalize(FSInputNormal);
    float specularStrength = 0.1;

	if(textureColor.a <= ALPHA_BIAS)
		discard;

	FSOutFragColor.a = 1.0;

	FSOutFragColor = AmbientColor;

    vec3 lightDir = normalize(-LightDirection);
    float lightIntensity = clamp(dot(normal, lightDir), 0.0, 1.0);

    if (lightIntensity > 0.0)
        FSOutFragColor += lightIntensity * DiffuseColor;

    vec3 viewDir = normalize(CameraPosition - FSInputWorldPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfwayDir), 0.0), SpecularPower);
    
    FSOutFragColor = clamp(FSOutFragColor, 0.0, 1.0);

    FSOutFragColor = FSOutFragColor * textureColor;

    FSOutFragColor += specularStrength * specular * DiffuseColor;
    FSOutFragColor = FSOutFragColor;
}