#pragma once

#include <glm/glm.hpp>
#include "Texture3D.h"

class WorleyNoise
{
private:

	       const int COMPUTE_SHADER_BLOCKS_COUNT = 8;
	
	static const int MIN_MAX_BUFFER_VALUE        = 100000000;

	static const int CELLS_A_WIDTH               = 2;
	static const int CELLS_B_WIDTH               = 3;
	static const int CELLS_C_WIDTH               = 4;

	static const int NUM_CELLS_A                 = CELLS_A_WIDTH * CELLS_A_WIDTH * CELLS_A_WIDTH;
	static const int NUM_CELLS_B                 = CELLS_B_WIDTH * CELLS_B_WIDTH * CELLS_B_WIDTH;
	static const int NUM_CELLS_C                 = CELLS_C_WIDTH * CELLS_C_WIDTH * CELLS_C_WIDTH;

public:

	struct NoiseParameters
	{
		int   TextureSize;
		int   Tiles;
		float Persistance;
	};

private:

	struct PointsPositions
	{
	public:

		glm::vec4 PointsA[NUM_CELLS_A];
		glm::vec4 PointsB[NUM_CELLS_B];
		glm::vec4 PointsC[NUM_CELLS_C];
	};

	struct MinMaxValues
	{
	public:

		int Mn;
		int Mx;
	};

public:

	WorleyNoise();
	~WorleyNoise();

	Texture3D* RenderNoise(NoiseParameters);

private:

	void CreatePointsPositions();
	void CreatePointsPositionsSet(glm::vec4*, int);
	
	void CreatePointsPositionsBuffer();
	void FreePointsPositionsBuffer();

	void CreateMinMaxBuffer();
	void FreeMinMaxBuffer();

private:

	PointsPositions m_pointPositions;

	unsigned int    m_pointsPositionsBuffer;
	unsigned int    m_minMaxBuffer;
};