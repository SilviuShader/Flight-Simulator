#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ReflectionCamera.h"

using namespace glm;

ReflectionCamera::ReflectionCamera(Camera* camera, float reflectionSurfaceheight) :
	m_camera(camera),
	m_reflectionSurfaceHeight(reflectionSurfaceheight)
{
    Update(0.0f);
}

void ReflectionCamera::UpdateWindowSize(float width, float height)
{
}

void ReflectionCamera::Update(float deltaTime)
{
    m_directionVectors = m_camera->GetReflectedVectors();

	UpdateViewMatrix();
    UpdateModelMatrix();
}

mat4 ReflectionCamera::GetModelMatrix() const
{
    return m_modelMatrix;
}

mat4 ReflectionCamera::GetViewMatrix() const
{
    return m_viewMatrix;
}

mat4 ReflectionCamera::GetProjectionMatrix() const
{
    return m_camera->GetProjectionMatrix();
}

vec3 ReflectionCamera::GetPosition() const
{
    float distToPlane = m_camera->GetPosition().y - m_reflectionSurfaceHeight;
    vec3 eye = m_camera->GetPosition() - (vec3(0.0f, 1.0f, 0.0f) * 2.0f * distToPlane);
    return eye;
}

float ReflectionCamera::GetWidth() const
{
    return m_camera->GetWidth();
}

float ReflectionCamera::GetHeight() const
{
    return m_camera->GetHeight();
}

float ReflectionCamera::GetAspectRatio() const
{
    return m_camera->GetAspectRatio();
}

float ReflectionCamera::GetFieldOfViewY() const
{
    return m_camera->GetFieldOfViewY();
}

float ReflectionCamera::GetNear() const
{
    return m_camera->GetNear();
}

float ReflectionCamera::GetFar() const
{
    return m_camera->GetFar();
}

vec3 ReflectionCamera::GetForward() const
{
    return m_directionVectors.Forward;
}

vec3 ReflectionCamera::GetRight() const
{
    return m_directionVectors.Right;
}

vec3 ReflectionCamera::GetUp() const
{
    return m_directionVectors.Up;
}

void ReflectionCamera::UpdateViewMatrix()
{
    vec3 up   = GetUp();
    vec3 eye  = GetPosition();
    vec3 look = eye + GetForward();

    m_viewMatrix = lookAt(eye, look, up);
}

void ReflectionCamera::UpdateModelMatrix()
{
    m_modelMatrix = inverse(m_viewMatrix);
}
