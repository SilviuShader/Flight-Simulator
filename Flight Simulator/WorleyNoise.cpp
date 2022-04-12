#include "WorleyNoise.h"
#include "ShaderManager.h"
#include "glad/glad.h"
#include "MathHelper.h"

using namespace glm;

WorleyNoise::WorleyNoise()
{
	CreatePointsPositions();
	CreatePointsPositionsBuffer();
	CreateMinMaxBuffer();
}

WorleyNoise::~WorleyNoise()
{
	FreeMinMaxBuffer();
	FreePointsPositionsBuffer();
}

Texture3D* WorleyNoise::RenderNoise(NoiseParameters noiseParameters)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        noiseShader   = shaderManager->GetWorleyNoiseShader();
	Texture3D*     noiseTexture  = new Texture3D(noiseParameters.TextureSize,
		                                         noiseParameters.TextureSize,
		                                         noiseParameters.TextureSize);

	noiseShader->Use();

	noiseShader->SetImage3D("ImgOutput",                   noiseTexture, 0, Texture::Format::RGBA32F);

	noiseShader->SetUniformBlockBinding("PointsPositions", 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_pointsPositionsBuffer);

	noiseShader->SetShaderStorageBlockBinding("MinMaxValues", 2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_minMaxBuffer);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT));

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	Shader* texture3DNormalizeShader = shaderManager->GetTexture3DNormalizeShader();

	texture3DNormalizeShader->Use();

	texture3DNormalizeShader->SetImage3D("ImgOutput",         noiseTexture, 0, Texture::Format::RGBA32F);
	noiseShader->SetShaderStorageBlockBinding("MinMaxValues", 1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_minMaxBuffer);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT));

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	return noiseTexture;
}

void WorleyNoise::CreatePointsPositions()
{
	CreatePointsPositionsSet(m_pointPositions.PointsA, CELLS_A_WIDTH);
	CreatePointsPositionsSet(m_pointPositions.PointsB, CELLS_B_WIDTH);
	CreatePointsPositionsSet(m_pointPositions.PointsC, CELLS_C_WIDTH);
}

void WorleyNoise::CreatePointsPositionsSet(vec4* points, int width)
{
	for (int z = 0; z < width; z++)
	{
		for (int y = 0; y < width; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float slice = 1.0f / (float)width;

				float minX = x * slice;
				float maxX = (x + 1) * slice;

				float minY = y * slice;
				float maxY = (y + 1) * slice;

				float minZ = z * slice;
				float maxZ = (z + 1) * slice;

				int index = (z * width * width + y * width + x);

				points[index] = vec4(MathHelper::RandomFloat(minX, maxX),
					                 MathHelper::RandomFloat(minY, maxY),
					                 MathHelper::RandomFloat(minZ, maxZ),
					                 1.0f);
			}
		}
	}
}

void WorleyNoise::CreatePointsPositionsBuffer()
{
	glGenBuffers(1, &m_pointsPositionsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_pointsPositionsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_pointPositions), &m_pointPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void WorleyNoise::FreePointsPositionsBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &m_pointsPositionsBuffer);
}

void WorleyNoise::CreateMinMaxBuffer()
{
	MinMaxValues data;
	data.Mn = MIN_MAX_BUFFER_VALUE;
	data.Mx = 0;

	glGenBuffers(1, &m_minMaxBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_minMaxBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
}

void WorleyNoise::FreeMinMaxBuffer()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &m_minMaxBuffer);
}
