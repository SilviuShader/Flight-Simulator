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

		float GetSignedDistanceToPlane(const glm::vec3&) const;

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

	struct BoundingVolume
	{
	public:

		virtual bool IsOnFrustum(const Frustum&) const = 0;
	};

	struct AABB : public BoundingVolume
	{
	public:

		AABB();
		AABB(const glm::vec3&, const glm::vec3&);
		AABB(const glm::vec3&, float, float, float);

		bool IsOnFrustum(const Frustum&)      const override;
		bool IsOnOrForwardPlane(const Plane&) const;

	public:

		glm::vec3 Center;
		glm::vec3 Extents;
	};

public:

	static Frustum GetCameraFrustum(Camera*);
};