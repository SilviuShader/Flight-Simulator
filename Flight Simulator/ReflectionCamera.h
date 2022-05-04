#pragma once

#include "Camera.h"

class ReflectionCamera : public Camera
{
public:

	ReflectionCamera(Camera*, float);

	void      UpdateWindowSize(float, float) override;

	void      Update(float)                  override;
							        	     
	glm::mat4 GetModelMatrix()      const    override;
	glm::mat4 GetViewMatrix()       const    override;
	glm::mat4 GetProjectionMatrix() const    override;
	glm::vec3 GetPosition()         const    override;
										     
	float     GetWidth()            const    override;
	float     GetHeight()           const    override;
	float     GetAspectRatio()      const    override;
	float     GetFieldOfViewY()     const    override;
										     
	float     GetNear()             const    override;
	float     GetFar()              const    override;
										     
	glm::vec3 GetForward()          const    override;
	glm::vec3 GetRight()            const    override;
	glm::vec3 GetUp()               const    override;

private:

	void UpdateViewMatrix();
	void UpdateModelMatrix();

private:

	Camera*            m_camera;
	float              m_reflectionSurfaceHeight;
					   
	glm::mat4          m_modelMatrix;
	glm::mat4          m_viewMatrix;

	Camera::Directions m_directionVectors;
};