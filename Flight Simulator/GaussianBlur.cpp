#include "glad/glad.h"
#include "GaussianBlur.h"
#include "ShaderManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

GaussianBlur::GaussianBlur(float blurAmount) :
	m_blurAmount(blurAmount)
{
	CreateOffsetsWeightsBuffer();
}

GaussianBlur::~GaussianBlur()
{
	FreeOffsetsWeightsBuffer();
}

void GaussianBlur::ApplyBlur(Texture* texture)
{
	ShaderManager* shaderManager      = ShaderManager::GetInstance();
	Shader*        gaussianBlurShader = shaderManager->GetGaussianBlurShader();

	gaussianBlurShader->Use();

	gaussianBlurShader->SetImage2D("ImgOutput", texture, 0, Texture::Format::R32F);
	gaussianBlurShader->SetInt("BorderSize", 5);

	SetBlurShaderParameters(vec2(1.0f, 0.0f), gaussianBlurShader);
	glDispatchCompute(Texture::GetComputeShaderGroupsCount(texture->GetWidth(), BLOCKS_COUNT), Texture::GetComputeShaderGroupsCount(texture->GetWidth(), BLOCKS_COUNT), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	SetBlurShaderParameters(vec2(0.0f, 1.0f), gaussianBlurShader);
	glDispatchCompute(Texture::GetComputeShaderGroupsCount(texture->GetWidth(), BLOCKS_COUNT), Texture::GetComputeShaderGroupsCount(texture->GetWidth(), BLOCKS_COUNT), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GaussianBlur::SetBlurShaderParameters(vec2 direction, Shader* shader)
{
	float sampleWeights[SAMPLE_COUNT];
	vec2  sampleOffsets[SAMPLE_COUNT];

	sampleWeights[0] = ComputeGaussian(0.0f);
	sampleOffsets[0] = vec2(0.0f, 0.0f);

	float totalWeights = sampleWeights[0];

	for (int i = 0; i < SAMPLE_COUNT / 2; i++)
	{
		float weight = ComputeGaussian(i + 1);

		sampleWeights[i * 2 + 1] = weight;
		sampleWeights[i * 2 + 2] = weight;

		totalWeights += weight * 2;

		float sampleOffset = i;
		vec2 delta = direction * sampleOffset;

		sampleOffsets[i * 2 + 1] = delta;
		sampleOffsets[i * 2 + 2] = -delta;
	}

	vec4 offsetsWeights[SAMPLE_COUNT];

	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		sampleWeights[i] /= totalWeights;
		offsetsWeights[i] = vec4(sampleOffsets[i].x, sampleOffsets[i].y, sampleWeights[i], 0.0f);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_offsetsWeightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * SAMPLE_COUNT, offsetsWeights, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	shader->SetUniformBlockBinding("OffsetsWeights", 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_offsetsWeightsBuffer);
}

float GaussianBlur::ComputeGaussian(float n)
{
	float theta = m_blurAmount;

	return (float)(1.0 / sqrtf(2 * pi<float>() * theta) *
		expf(-(n * n) / (2 * theta * theta)));
}

void GaussianBlur::CreateOffsetsWeightsBuffer()
{
	glGenBuffers(1, &m_offsetsWeightsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_offsetsWeightsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GaussianBlur::FreeOffsetsWeightsBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &m_offsetsWeightsBuffer);
}
