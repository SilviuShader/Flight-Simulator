#version 430 core

in vec2 FSInputTexCoords;

uniform sampler2D SceneTexture;
uniform sampler2D DepthTexture;

uniform mat4  CameraMatrix;
uniform float AspectRatio;
uniform float Near;
uniform float Far;
uniform float FovY;

out vec4 FSOutFragColor;

float linearizeDepth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 rayDirection)
{
	vec3 inverseRayDir = 1.0 / rayDirection;
	vec3 t0 = (boundsMin - rayOrigin) * inverseRayDir;
	vec3 t1 = (boundsMax - rayOrigin) * inverseRayDir;

	vec3 tmin = min(t0, t1);
	vec3 tmax = max(t0, t1);

	float distA = max(max(tmin.x, tmin.y), tmin.z);
	float distB = min(min(tmax.x, tmax.y), tmax.z);

	float distToBox = max(0, distA);
	float distInsideBox = max(0, distB - distToBox);

	return vec2(distToBox, distInsideBox);
}

void main()
{
	vec3 sceneColor         = texture(SceneTexture, FSInputTexCoords).xyz;
	vec3 invertedSceneColor = 1.0 - sceneColor;

	vec2 normalizeCoords    = FSInputTexCoords * 2.0 - vec2(1.0, 1.0);
	     normalizeCoords.x *= AspectRatio;
		 normalizeCoords   *= tan(FovY * 0.5) * Near;

	vec3 onCameraPoint      = vec3(normalizeCoords.x, normalizeCoords.y, -Near);
	vec3 eye                = vec3(0.0, 0.0, 0.0);
	vec3 rayDirection       = onCameraPoint - eye;
	     eye                = (CameraMatrix * vec4(eye,          1.0)).xyz;
		 rayDirection       = (CameraMatrix * vec4(rayDirection, 0.0)).xyz;
	     onCameraPoint      = eye + rayDirection;

	vec4  depthColor        = texture(DepthTexture, FSInputTexCoords);
	float depth             = linearizeDepth(depthColor.x, Near, Far) * (length(rayDirection) / Near);

	     rayDirection       = normalize(rayDirection);

	vec2 boxIntersectDetails = rayBoxDst(vec3(-10.0, -10.0, -10.0), vec3(10.0, 10.0, 10.0), onCameraPoint, rayDirection);

	if (boxIntersectDetails.y >= 0.001 && boxIntersectDetails.x < depth)
		FSOutFragColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		FSOutFragColor = vec4(sceneColor, 1.0) + depthColor * 0.001;
}