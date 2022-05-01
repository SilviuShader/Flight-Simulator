#pragma once
#include "Texture.h"
#include "Camera.h"
#include "WorleyNoise.h"
#include "Light.h"

class Clouds
{
public:

	Clouds();
	~Clouds();

	void Draw(Camera*, Light*, Texture*, Texture*);

private:

	WorleyNoise* m_worleyNoise;
	Texture3D*   m_worleyNoiseTexture;
};