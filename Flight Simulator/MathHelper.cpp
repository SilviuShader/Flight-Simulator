#include "MathHelper.h"

using namespace glm;

MathHelper::Plane::Plane(vec3 pointOnPlane, vec3 normal) :
	Normal(normal),
	Distance(-dot(normal, pointOnPlane))
{
}

float MathHelper::Plane::GetSignedDistanceToPlane(const vec3& point)
{
	return dot(Normal, point) - Distance;
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
	return true;
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

	float halfVertical   = tanf(camera->GetFieldOfViewY()) * cameraFar;
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