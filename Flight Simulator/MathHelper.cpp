#include "MathHelper.h"
#include "Camera.h"

using namespace std;
using namespace glm;

MathHelper::Plane::Plane(vec3 pointOnPlane, vec3 normal) :
	Normal(normal),
	Distance(dot(normal, pointOnPlane))
{
}

float MathHelper::Plane::GetSignedDistanceToPlane(const vec3& point) const
{
	return dot(Normal, point) - Distance;
}

MathHelper::AABB::AABB() :
	Center(vec3(0.0f, 0.0f, 0.0f)),
	Extents(vec3(0.0f, 0.0f, 0.0f))
{
}

MathHelper::AABB::AABB(const vec3& min, const vec3& max) :
	BoundingVolume(),
	Center((min + max) * 0.5f),
	Extents(vec3(max.x - Center.x, max.y - Center.y, max.z - Center.z))
{
}

MathHelper::AABB::AABB(const vec3& center, float iI, float iJ, float iK) :
	BoundingVolume(),
	Center(center),
	Extents(vec3(iI, iJ, iK))
{
}

bool MathHelper::AABB::IsOnFrustum(const Frustum& frustum) const
{
	return IsOnOrForwardPlane(frustum.NearFace)  &&
		   IsOnOrForwardPlane(frustum.FarFace)   &&
		   IsOnOrForwardPlane(frustum.LeftFace)  &&
		   IsOnOrForwardPlane(frustum.RightFace) &&
		   IsOnOrForwardPlane(frustum.TopFace)   &&
		   IsOnOrForwardPlane(frustum.BottomFace);
}

bool MathHelper::AABB::IsOnOrForwardPlane(const Plane& plane) const
{
	const float r = Extents.x * abs(plane.Normal.x) +
		            Extents.y * abs(plane.Normal.y) +
					Extents.z * abs(plane.Normal.z);

	return -r <= plane.GetSignedDistanceToPlane(Center);
}

MathHelper::Frustum MathHelper::GetCameraFrustum(Camera* camera)
{
	Frustum result;

	vec3  cameraPosition = camera->GetPosition();
	vec3  cameraForward  = camera->GetForward();
	vec3  cameraRight    = camera->GetRight();
	vec3  cameraUp       = camera->GetUp();
	float cameraNear     = camera->GetNear();
	float cameraFar      = camera->GetFar();

	vec3 farForward      = cameraForward * cameraFar;

	float halfVertical   = tanf(camera->GetFieldOfViewY() * 0.5f) * cameraFar;
	float halfHorizontal = camera->GetAspectRatio() * halfVertical;

	result.NearFace   = Plane(cameraPosition + cameraForward * cameraNear,     cameraForward);
	result.FarFace    = Plane(cameraPosition + farForward,                     -cameraForward);

	result.LeftFace   = Plane(cameraPosition, 
		                      cross(farForward - cameraRight * halfHorizontal, cameraUp));
	result.RightFace  = Plane(cameraPosition, 
		                      cross(cameraUp,                                  farForward + cameraRight * halfHorizontal));

	result.TopFace    = Plane(cameraPosition, 
		                      cross(farForward + cameraUp * halfVertical,      cameraRight));
	result.BottomFace = Plane(cameraPosition, 
		                      cross(cameraRight,                               farForward - cameraUp * halfVertical));

	return result;
}

unsigned MathHelper::PowerCeil(unsigned x)
{
	if (x <= 1) 
		return 1;
	
	int power = 2;
	x--;

	while (x >>= 1) 
		power <<= 1;

	return power;
}

int MathHelper::GCD(int a, int b)
{
	if (b != 0)
		return GCD(b, a % b);
	else
		return a;
}

int MathHelper::LCM(int a, int b)
{
	return (a * b) / GCD(a, b);
}

//https://stackoverflow.com/questions/686353/random-float-number-generation
float MathHelper::RandomFloat(float minValue, float maxValue)
{
	return minValue + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maxValue - minValue)));;
}
