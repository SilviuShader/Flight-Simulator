#pragma once

#include <glm/glm.hpp>
#include "Camera.h"

class MathHelper
{
public:

	struct Plane
	{
	public:

		Plane(glm::vec3 = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 = glm::vec3(0.0f, 1.0f, 0.0f));

	public:

		glm::vec3 Normal;
		float     Distance;
	};

	struct Frustum
	{
	public:

		Plane TopFace;
		Plane BottomFace;

		Plane RightFace;
		Plane LeftFace;

		Plane FarFace;
		Plane NearFace;
	};

public:

	Frustum GetCameraFrustum(Camera*);
};