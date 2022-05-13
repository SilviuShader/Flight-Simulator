#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

class GaussianBlur
{
private:

	static const int SAMPLE_COUNT = 15;
	static const int BLOCKS_COUNT = 8;

public:

	GaussianBlur(float);
	~GaussianBlur();

	void ApplyBlur(Texture*);

private:

	void  SetBlurShaderParameters(glm::vec2, Shader*);
	float ComputeGaussian(float);

	void CreateOffsetsWeightsBuffer();
	void FreeOffsetsWeightsBuffer();

private:

	float        m_blurAmount;

	unsigned int m_offsetsWeightsBuffer;
};