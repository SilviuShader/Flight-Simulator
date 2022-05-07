#version 430 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 3) out;

in vec2 GSInputTexCoords[];
in vec4 GSInputWorldPosition[];
in vec4 GSInputReflectionPosition[];
in vec3 GSInputWaterToCamera[];

out vec2 FSInputTexCoords;
out vec4 FSInputReflectionPosition;
out vec3 FSInputWaterToCamera;
out vec3 FSInputNormal;
out vec3 FSInputTangent;
out vec3 FSInputBinormal;
out vec4 FSInputWorldPosition;

void main()
{
	vec3 a      = (GSInputWorldPosition[1] - GSInputWorldPosition[0]).xyz;
	vec3 b      = (GSInputWorldPosition[2] - GSInputWorldPosition[0]).xyz;
	vec3 normal = normalize(cross(a, b));

	vec3 u = vec3(1.0, 0.0, 0.0);
	vec3 v = vec3(0.0, 0.0, 1.0);
	float normSquared = dot(normal, normal);
	vec3 tangent = normalize(u - ((dot(u, normal) / normSquared) * normal));
	vec3 binormal = normalize(v - ((dot(v, normal) / normSquared) * normal));

	FSInputTexCoords          = GSInputTexCoords[0];
	FSInputReflectionPosition = GSInputReflectionPosition[0];
	FSInputWaterToCamera      = GSInputWaterToCamera[0];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[0];
	gl_Position               = gl_in[0].gl_Position;
	EmitVertex();

	FSInputTexCoords          = GSInputTexCoords[1];
	FSInputReflectionPosition = GSInputReflectionPosition[1];
	FSInputWaterToCamera      = GSInputWaterToCamera[1];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[1];
	gl_Position               = gl_in[1].gl_Position;
	EmitVertex();

	FSInputTexCoords          = GSInputTexCoords[2];
	FSInputReflectionPosition = GSInputReflectionPosition[2];
	FSInputWaterToCamera      = GSInputWaterToCamera[2];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[2];
	gl_Position               = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}