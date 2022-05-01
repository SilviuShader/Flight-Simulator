#pragma once
#include "Texture.h"
#include "Camera.h"
#include "WorleyNoise.h"
#include "Light.h"
#include "PerlinNoise.h"

class Clouds
{
public:

	Clouds();
	~Clouds();

	void Draw(Camera*, Light*, Texture*, Texture*);

private:

	WorleyNoise* m_worleyNoise;
	PerlinNoise* m_perlinNoise;
	
	Texture3D*   m_worleyNoiseTexture;
	Texture3D*   m_detailNoiseTexture;
	Texture*     m_weatherMap;
};