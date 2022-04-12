#include "WorleyNoise.h"
#include "ShaderManager.h"
#include "glad/glad.h"
#include "MathHelper.h"

using namespace glm;

WorleyNoise::WorleyNoise()
{
}

WorleyNoise::~WorleyNoise()
{
}

Texture3D* WorleyNoise::RenderNoise(NoiseParameters noiseParameters, Texture3D* noiseTexture)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        noiseShader   = shaderManager->GetWorleyNoiseShader();

	if (!noiseTexture)
		noiseTexture  = new Texture3D(noiseParameters.TextureSize,
					                  noiseParameters.TextureSize,
					                  noiseParameters.TextureSize);

	int   pointsCount = noiseParameters.NumCellsA * noiseParameters.NumCellsA * noiseParameters.NumCellsA +
		                noiseParameters.NumCellsB * noiseParameters.NumCellsB * noiseParameters.NumCellsB +
		                noiseParameters.NumCellsC * noiseParameters.NumCellsC * noiseParameters.NumCellsC;

	vec4* points      = new vec4[pointsCount];

	CreatePointsPositions(points, noiseParameters);

	unsigned int pointsPositionsBuffer = CreatePointsPositionsBuffer(points, pointsCount);
	unsigned int minMaxBuffer          = CreateMinMaxBuffer();

	noiseShader->Use();

	noiseShader->SetImage3D("ImgOutput",                   noiseTexture, 0, Texture::Format::RGBA32F);

	noiseShader->SetUniformBlockBinding("PointsPositions", 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, pointsPositionsBuffer);

	noiseShader->SetShaderStorageBlockBinding("MinMaxValues", 2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, minMaxBuffer);

	noiseShader->SetInt("Tiles",         noiseParameters.Tiles);
	noiseShader->SetFloat("Persistance", noiseParameters.Persistance);

	noiseShader->SetInt("NumCellsA",     noiseParameters.NumCellsA);
	noiseShader->SetInt("NumCellsB",     noiseParameters.NumCellsB);
	noiseShader->SetInt("NumCellsC",     noiseParameters.NumCellsC);

	noiseShader->SetVec4("ChannelsMask", noiseParameters.ChannelsMask);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT));

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	Shader* texture3DNormalizeShader = shaderManager->GetTexture3DNormalizeShader();

	texture3DNormalizeShader->Use();

	texture3DNormalizeShader->SetImage3D("ImgOutput",         noiseTexture, 0, Texture::Format::RGBA32F);
	noiseShader->SetShaderStorageBlockBinding("MinMaxValues", 1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, minMaxBuffer);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT));

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	FreeMinMaxBuffer(minMaxBuffer);
	FreePointsPositionsBuffer(pointsPositionsBuffer);

	if (points)
	{
		delete[] points;
		points = nullptr;
	}

	return noiseTexture;
}

void WorleyNoise::CreatePointsPositions(vec4* points, const NoiseParameters& noiseParameters)
{
	int totalAPoints = noiseParameters.NumCellsA * noiseParameters.NumCellsA * noiseParameters.NumCellsA;
	int totalBPoints = noiseParameters.NumCellsB * noiseParameters.NumCellsB * noiseParameters.NumCellsB;

	CreatePointsPositionsSet(&points[0],                           noiseParameters.NumCellsA);
	CreatePointsPositionsSet(&points[totalAPoints],                noiseParameters.NumCellsB);
	CreatePointsPositionsSet(&points[totalAPoints + totalBPoints], noiseParameters.NumCellsC);
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

unsigned int WorleyNoise::CreatePointsPositionsBuffer(vec4* points, int numPoints)
{
	unsigned int pointsPositionsBuffer;

	glGenBuffers(1, &pointsPositionsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, pointsPositionsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * numPoints, points, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return pointsPositionsBuffer;
}

void WorleyNoise::FreePointsPositionsBuffer(unsigned int pointsPositionsBuffer)
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &pointsPositionsBuffer);
}

unsigned int WorleyNoise::CreateMinMaxBuffer()
{
	unsigned int minMaxBuffer;

	MinMaxValues data;
	data.Mn = MIN_MAX_BUFFER_VALUE;
	data.Mx = 0;

	glGenBuffers(1, &minMaxBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, minMaxBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

	return minMaxBuffer;
}

void WorleyNoise::FreeMinMaxBuffer(unsigned int minMaxBuffer)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &minMaxBuffer);
}
