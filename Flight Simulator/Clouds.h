#pragma once
#include "Texture.h"
#include "Camera.h"
#include "WorleyNoise.h"
#include "Light.h"
#include "PerlinNoise.h"

class Clouds
{
public:

	struct CloudsProperties
	{
		glm::vec3 OffsetVelocity;
		glm::vec3 DetailsOffsetVelocity;
		glm::vec2 CloudBoxExtents;
		float     CloudsAltitude;
	};

public:

	Clouds(CloudsProperties);
	~Clouds();

	void Update(float);
	void Draw(Camera*, Light*, Texture*, Texture*);

private:

	void UpdateOffset(glm::vec3&, glm::vec3&, float);

private:

	CloudsProperties m_cloudsProperties;

	WorleyNoise*     m_worleyNoise;
	PerlinNoise*     m_perlinNoise;
				     
	Texture3D*       m_worleyNoiseTexture;
	Texture3D*       m_detailNoiseTexture;
	Texture*         m_weatherMap;
				     
	glm::vec3        m_cloudsOffset;
	glm::vec3        m_detailsOffset;
};