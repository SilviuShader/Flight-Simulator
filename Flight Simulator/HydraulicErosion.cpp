#include <random>
#include <functional>
#include <algorithm>

#include "glad/glad.h"
#include "HydraulicErosion.h"
#include "ShaderManager.h"

using namespace std;
using namespace glm;

HydraulicErosion::HydraulicErosion(ErosionParameters parameters) :
	m_parameters(parameters)
{
	CreateRandomIndices();
	CreateErosionBrushDetails();

	CreateRandomIndicesBuffer();
	CreateErosionBrushDetailsBuffer();
}

HydraulicErosion::~HydraulicErosion()
{
	FreeErosionBrushDetailsBuffer();
	FreeRandomIndicesBuffer();
}

void HydraulicErosion::ApplyErosion(Texture* heightMap)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        erosionShader = shaderManager->GetHydraulicErosionShader();

	erosionShader->Use();

	erosionShader->SetImage2D("ImgOutput", heightMap, 0, Texture::Format::R32F);

	erosionShader->SetUniformBlockBinding("RandomIndices", 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_randomIndicesBuffer);

	erosionShader->SetUniformBlockBinding("ErosionBrushDetails", 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_erosionBrushDetailsBuffer);

	erosionShader->SetInt("DropletLifetime", 90);
	erosionShader->SetInt("BrushWidth", m_parameters.BrushWidth);

	erosionShader->SetFloat("InitialWater",           1);
	erosionShader->SetFloat("EvaporateSpeed",         0.02);
	erosionShader->SetFloat("InitialSpeed",           2.0);
	erosionShader->SetFloat("SedimentCapacityFactor", 3.0f);
	erosionShader->SetFloat("MinSedimentCapacity",    0.01);
	erosionShader->SetFloat("DepositSpeed",           0.4f);
	erosionShader->SetFloat("ErodeSpeed",             0.4f);
	erosionShader->SetFloat("Gravity",                4.0f);
	erosionShader->SetFloat("Inertia",                0.7f);
	erosionShader->SetInt("BorderSize",               10);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(m_parameters.Iterations, THREADS_PER_BLOCK), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void HydraulicErosion::CreateRandomIndices()
{
	mt19937 generator(m_parameters.Seed);
	uniform_int_distribution<int> distribution;
	auto random = bind(distribution, generator);

	int totalThreads = Texture::GetComputeShaderGroupsCount(m_parameters.Iterations, THREADS_PER_BLOCK) * THREADS_PER_BLOCK;

	for (int i = 0; i < MAX_RANDOM_INDICES; i++)
		m_randomIndices[i].Index = random();
}

void HydraulicErosion::CreateErosionBrushDetails()
{
	int centerX = m_parameters.BrushWidth / 2;
	int centerY = m_parameters.BrushWidth / 2;

	float radius = (float)m_parameters.BrushWidth / 2.0f;

	int index = 0;
	float totalWeights = 0.0f;

	for (int x = 0; x < m_parameters.BrushWidth; x++)
	{
		for (int y = 0; y < m_parameters.BrushWidth; y++)
		{
			int dx = x - centerX;
			int dy = y - centerY;

			float weight = sqrtf(dx * dx + dy * dy) / radius;
			weight = std::min(weight, 1.0f);
			weight = 1.0f - weight;

			totalWeights += weight;

			m_erosionBrushDetails[index++] = vec4(dx, dy, weight, 0.0f);
		}
	}

	for (int i = 0; i < index; i++)
	{
		vec4 oldVal = m_erosionBrushDetails[i];
		m_erosionBrushDetails[i] = vec4(oldVal.x, oldVal.y, oldVal.z / totalWeights, oldVal.w);
	}
}

void HydraulicErosion::CreateRandomIndicesBuffer()
{
	glGenBuffers(1, &m_randomIndicesBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_randomIndicesBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(RandomIndices) * MAX_RANDOM_INDICES, m_randomIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void HydraulicErosion::FreeRandomIndicesBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &m_randomIndicesBuffer);
}

void HydraulicErosion::CreateErosionBrushDetailsBuffer()
{
	glGenBuffers(1, &m_erosionBrushDetailsBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_erosionBrushDetailsBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * MAX_BRUSH_SIZE, m_erosionBrushDetails, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void HydraulicErosion::FreeErosionBrushDetailsBuffer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &m_erosionBrushDetailsBuffer);
}
