#version 430 core

in vec2 FSInputTexCoords;

uniform sampler2D SceneTexture;
uniform sampler2D DepthTexture;
uniform sampler3D CloudsDensityTexture;
uniform sampler3D DetailNoiseTexture;
uniform sampler2D WeatherMap;

uniform mat4      CameraMatrix;
uniform float     AspectRatio;
uniform float     Near;
uniform float     Far;
uniform float     FovY;
			      
uniform vec3      BoundsMin;
uniform vec3      BoundsMax;

uniform vec3      CloudScale;
uniform float     DetailNoiseScale;
uniform vec3      CloudOffset;
uniform float     DensityMultiplier;
uniform float     DarknessThreshold;
uniform float     DensityOffset;
uniform vec4      PhaseParams;
uniform int       FocusedEyeSunExponent;
uniform vec4      ShapeNoiseWeights;
uniform vec4      DetailNoiseWeights;
uniform float     LightAbsorbtionTowardSun;
uniform float     LightAbsorptionThroughCloud;
uniform float     DetailNoiseWeight;
uniform float     RayMarchStepSize;
			      
uniform vec4      DiffuseColor;
uniform vec3      LightDirection;

uniform int       LightStepsCount;

out vec4 FSOutFragColor;

float linearizeDepth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

float beer(float d)
{
	return exp(-d);
}

float hg(float a, float g) 
{
    float g2 = g*g;
    return (1-g2) / (4*3.1415*pow(1+g2-2*g*(a), 1.5));
}

float phase(float a) 
{
    float blend = .5;
    float hgBlend = hg(a,PhaseParams.x) * (1-blend) + hg(a,-PhaseParams.y) * blend;
    return PhaseParams.z + hgBlend*PhaseParams.w;
}

float remap(float x, float oldMin, float oldMax, float newMin, float newMax)
{
    float oldDiff = oldMax - oldMin;
	x = (x - oldMin) / oldDiff;
	return newMin + x * (newMax - newMin);
}

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
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

float sampleDensity(vec3 position)
{
	vec3 size = BoundsMax - BoundsMin;
	vec3 boundsCenter = (BoundsMin + BoundsMax) * 0.5;
	vec3 uvw = (size * 0.5 + position) * CloudScale;
	vec3 shapeSamplePos = uvw + CloudOffset;

	float containerEdgeFadeDst = 50;
	float dstFromEdgeX = min(containerEdgeFadeDst, min(position.x - BoundsMin.x, BoundsMax.x - position.x));
	float dstFromEdgeZ = min(containerEdgeFadeDst, min(position.z - BoundsMin.z, BoundsMax.z - position.z));

	float edgeWeight = min(dstFromEdgeX, dstFromEdgeZ) / containerEdgeFadeDst;

	vec2 weatherUV = (size.xz * 0.5 + (position.xz - boundsCenter.xz)) / max(size.x, size.z);
	float weatherMap = texture(WeatherMap, weatherUV).x;
	float gMin = remap(weatherMap, 0, 1, 0.1, 0.5);
	float gMax = remap(weatherMap, 0, 1, gMin, 0.9);
	float heightPercentage = (position.y - BoundsMin.y) / size.y;
	float heightGradient =  saturate(remap(heightPercentage, 0.0, gMin, 0, 1)) * saturate(remap(heightPercentage, 1, gMax, 0, 1));
	heightGradient *= edgeWeight;

	vec4 shapeNoise = texture(CloudsDensityTexture, shapeSamplePos);
	vec4 normalizedShapeWeights = ShapeNoiseWeights / dot(ShapeNoiseWeights, vec4(1.0, 1.0, 1.0, 1.0));
	float baseShapeRawDensity = dot(shapeNoise, normalizedShapeWeights) * heightGradient;
	float baseShapeDensity = baseShapeRawDensity + DensityOffset;

	if (baseShapeDensity > 0.0)
	{
		vec3 detailSamplePos = uvw * DetailNoiseScale;
		vec4 detailNoise = texture(DetailNoiseTexture, detailSamplePos);
		vec4 normalizedDetailWeights = DetailNoiseWeights / dot(DetailNoiseWeights, vec4(1.0, 1.0, 1.0, 1.0));
		float detailRawIntensity = dot(detailNoise, normalizedDetailWeights);

		float oneMinusShape = 1.0 - baseShapeRawDensity;
		float detailErodeWeight = oneMinusShape * oneMinusShape * oneMinusShape;
		float cloudDensity = baseShapeDensity - (1 - detailRawIntensity) * detailErodeWeight * DetailNoiseWeight;
	
		float finalDensity = cloudDensity * DensityMultiplier;
		
		return finalDensity;
	}

	return 0.0;
}

float lightMarch(vec3 pos)
{
	vec3  toLight = -normalize(LightDirection);
	float distInsideBox = rayBoxDst(BoundsMin, BoundsMax, pos, toLight).y;

	float stepSize = distInsideBox / float(LightStepsCount);
	float distanceAccumulated = 0.0;
	float maxDistance = distInsideBox;
	float totalDensity = 0.0;

	while (distanceAccumulated < maxDistance)
	{
		float density = sampleDensity(pos + toLight * distanceAccumulated);
		totalDensity += max(0, density * stepSize);
		distanceAccumulated = distanceAccumulated + stepSize;
	}

	float transmittance = beer(totalDensity * LightAbsorbtionTowardSun);

	transmittance = DarknessThreshold + transmittance * (1.0 - DarknessThreshold);

	return transmittance;
}

void main()
{
	vec3 sceneColor = texture(SceneTexture, FSInputTexCoords).xyz;
	vec3 invertedSceneColor = 1.0 - sceneColor;

	vec2 normalizeCoords = FSInputTexCoords * 2.0 - vec2(1.0, 1.0);
	normalizeCoords.x *= AspectRatio;
	normalizeCoords *= tan(FovY * 0.5) * Near;

	vec3 onCameraPoint = vec3(normalizeCoords.x, normalizeCoords.y, -Near);
	vec3 eye = vec3(0.0, 0.0, 0.0);
	vec3 rayDirection = onCameraPoint - eye;
	eye = (CameraMatrix * vec4(eye, 1.0)).xyz;
	rayDirection = (CameraMatrix * vec4(rayDirection, 0.0)).xyz;
	onCameraPoint = eye + rayDirection;

	vec4 depthColor = texture(DepthTexture, FSInputTexCoords);
	float depth = linearizeDepth(depthColor.x, Near, Far) * (length(rayDirection) / Near);

	rayDirection = normalize(rayDirection);

	vec2 boxIntersectDetails = rayBoxDst(BoundsMin, BoundsMax, onCameraPoint, rayDirection);

	float distToBox = boxIntersectDetails.x;
	float distInsideBox = boxIntersectDetails.y;

	float distanceAccumulated = 0.0;
	float maxDistance = min(distInsideBox, depth - distToBox);

	float transmittance = 1.0;
	float lightEnergy = 0.0;

	vec3  toLight = -normalize(LightDirection);
	float sunDot = dot(rayDirection, toLight);
	float phaseVal = phase(sunDot);

	while (distanceAccumulated < maxDistance)
	{
		vec3 currentPosition = onCameraPoint + rayDirection * (distToBox + distanceAccumulated);

		float density = sampleDensity(currentPosition);

		if (density > 0)
		{
			float lightTransmittance = lightMarch(currentPosition);
			lightEnergy += density * transmittance * lightTransmittance * RayMarchStepSize * phaseVal;

			transmittance *= exp(-density * RayMarchStepSize * LightAbsorptionThroughCloud);

			if (transmittance < 0.01)
				break;
		}

		distanceAccumulated += RayMarchStepSize;
	}

	float focusedEyeCos = pow(clamp(sunDot, 0.0, 1.0), FocusedEyeSunExponent);
	float sun = clamp(hg(focusedEyeCos, 0.9995), 0.0, 1.0) * transmittance;

	vec3 cloudCol = lightEnergy * DiffuseColor.xyz;
	vec3 col = (sceneColor * transmittance) + cloudCol;
	col = clamp(col, 0.0, 1.0) * (1 - sun) + DiffuseColor.xyz * sun;

	FSOutFragColor = vec4(col, 1.0);
}