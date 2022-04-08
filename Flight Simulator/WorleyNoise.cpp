#include "WorleyNoise.h"
#include "ShaderManager.h"
#include "glad/glad.h"

WorleyNoise::WorleyNoise()
{
}

WorleyNoise::~WorleyNoise()
{
}

Texture3D* WorleyNoise::RenderNoise(NoiseParameters noiseParameters)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        noiseShader   = shaderManager->GetWorleyNoiseShader();
	Texture3D*     noiseTexture  = new Texture3D(noiseParameters.TextureSize,
		                                         noiseParameters.TextureSize,
		                                         noiseParameters.TextureSize);

	noiseShader->Use();

	noiseShader->SetImage3D("ImgOutput", noiseTexture, 0, Texture::Format::RGBA32F);

	glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
		              Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT));

	return noiseTexture;
}
