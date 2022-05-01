#pragma once

#include <glm/glm.hpp>
#include "Texture3D.h"

class WorleyNoise
{
private:

	const int COMPUTE_SHADER_BLOCKS_COUNT = 8;

public:

	struct NoiseParameters
	{
		int       TextureSize;
		int       Tiles;
		float     Persistance;
			      
		int       NumCellsA;
		int       NumCellsB;
		int       NumCellsC;

		glm::vec4 ChannelsMask;
	};

public:

	WorleyNoise();
	~WorleyNoise();

	Texture3D* RenderNoise(NoiseParameters, Texture3D* = nullptr);

private:

	void         CreatePointsPositions(glm::vec4*, const NoiseParameters&);
	void         CreatePointsPositionsSet(glm::vec4*, int);
	
	unsigned int CreatePointsPositionsBuffer(glm::vec4*, int);
	void         FreePointsPositionsBuffer(unsigned int);
};