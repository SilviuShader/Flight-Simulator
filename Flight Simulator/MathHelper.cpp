#include "MathHelper.h"

using namespace glm;

MathHelper::Plane::Plane(vec3 pointOnPlane, vec3 normal) :
	Normal(normal),
	Distance(-dot(normal, pointOnPlane))
{

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

	// TODO: Check if the signs are right here (+cameraUp, -cameraUp)
	result.BottomFace = Plane(cameraPosition, 
		                      cross(farForward + cameraUp * halfVertical,      cameraRight));
	result.TopFace    = Plane(cameraPosition, 
		                      cross(cameraRight,                               farForward - cameraUp * halfVertical));

	return result;
}