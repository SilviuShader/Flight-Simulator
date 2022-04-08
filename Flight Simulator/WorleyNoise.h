#pragma once

#include "Texture3D.h"

class WorleyNoise
{
private:

	const int COMPUTE_SHADER_BLOCKS_COUNT = 8;

public:

	struct NoiseParameters
	{
		int TextureSize;
	};

public:

	WorleyNoise();
	~WorleyNoise();

	Texture3D* RenderNoise(NoiseParameters);
};