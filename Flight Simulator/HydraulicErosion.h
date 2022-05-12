#pragma once

#include "Texture.h"
#include <glm/glm.hpp>

class HydraulicErosion
{
private:

	static const int THREADS_PER_BLOCK  = 1024;
	static const int MAX_RANDOM_INDICES = 4096;
	static const int MAX_BRUSH_WIDTH    = 9;
	static const int MAX_BRUSH_SIZE     = MAX_BRUSH_WIDTH * MAX_BRUSH_WIDTH;

public:

	struct ErosionParameters
	{
	public:

		int Iterations;
		int Seed;
		int BrushWidth;
	};

	struct RandomIndices
	{
	public:

		int   Index;
		float Padding1;
		float Padding2;
		float Padding3;
	};

public:

	HydraulicErosion(ErosionParameters);
	~HydraulicErosion();

	void ApplyErosion(Texture*);

private:

	void CreateRandomIndices();
	void CreateErosionBrushDetails();

	void CreateRandomIndicesBuffer();
	void FreeRandomIndicesBuffer();

	void CreateErosionBrushDetailsBuffer();
	void FreeErosionBrushDetailsBuffer();

private:

	ErosionParameters   m_parameters;

	RandomIndices       m_randomIndices[MAX_RANDOM_INDICES];
	glm::vec4           m_erosionBrushDetails[MAX_BRUSH_SIZE];

	unsigned int        m_randomIndicesBuffer;
	unsigned int        m_erosionBrushDetailsBuffer;
};