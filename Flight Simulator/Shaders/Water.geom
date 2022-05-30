#version 430 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 3) out;

in vec2 GSInputTexCoords[];
in vec4 GSInputWorldPosition[];
in vec4 GSInputReflectionPosition[];
in vec4 GSInputRealClipCoords[];

out vec2 FSInputTexCoords;
out vec4 FSInputReflectionPosition;
out vec3 FSInputNormal;
out vec3 FSInputTangent;
out vec3 FSInputBinormal;
out vec4 FSInputWorldPosition;
out vec4 FSInputRealClipCoords;

void main()
{
	vec3 a      = (GSInputWorldPosition[1] - GSInputWorldPosition[0]).xyz;
	vec3 b      = (GSInputWorldPosition[2] - GSInputWorldPosition[0]).xyz;
	vec3 normal = normalize(cross(a, b));

	vec3 u = vec3(1.0, 0.0, 0.0);
	vec3 v = vec3(0.0, 0.0, 1.0);
	float normSquared = dot(normal, normal);

	// we can't use the 'a' and 'b' variables as tangents/ bionormals because they are not perpendicular to eachother.
	// that's why we must project the default tangents/ binormal onto our plane so we can get the correct ones.
	vec3 tangent = normalize(u - ((dot(u, normal) / normSquared) * normal));
	vec3 binormal = normalize(v - ((dot(v, normal) / normSquared) * normal));

	// instead of using a for loop, the 3 vertices are emitted manually so we can reduce the number of branch instructions.
	FSInputTexCoords          = GSInputTexCoords[0];
	FSInputReflectionPosition = GSInputReflectionPosition[0];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[0];
	FSInputRealClipCoords     = GSInputRealClipCoords[0];
	gl_Position               = gl_in[0].gl_Position;
	EmitVertex();

	FSInputTexCoords          = GSInputTexCoords[1];
	FSInputReflectionPosition = GSInputReflectionPosition[1];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[1];
	FSInputRealClipCoords     = GSInputRealClipCoords[1];
	gl_Position               = gl_in[1].gl_Position;
	EmitVertex();

	FSInputTexCoords          = GSInputTexCoords[2];
	FSInputReflectionPosition = GSInputReflectionPosition[2];
	FSInputNormal             = normal;
	FSInputTangent            = tangent;
	FSInputBinormal           = binormal;
	FSInputWorldPosition      = GSInputWorldPosition[2];
	FSInputRealClipCoords     = GSInputRealClipCoords[2];
	gl_Position               = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}