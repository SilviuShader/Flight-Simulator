#include <functional>
#include <cmath>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"
#include "glad/glad.h"
#include "VertexTypes.h"
#include "ShaderManager.h"

using namespace std;
using namespace glm;

PerlinNoise::PerlinNoise(int seed)
{
    GenerateNoiseValues(seed);
    CreateValuesBuffer();
}

PerlinNoise::~PerlinNoise()
{
    FreeValuesBuffer();
}

void PerlinNoise::GenerateNoiseValues(int seed)
{
    mt19937 generator(seed);
    uniform_real_distribution<float> distribution(0.0f, 2.0f * pi<float>());
    auto random = bind(distribution, generator);

    assert((SAMPLES_COUNT & (SAMPLES_COUNT - 1)) == 0);

    const int permutationsSize = SAMPLES_COUNT << 1;
    const int permutationsMask = SAMPLES_COUNT - 1;
    int permutationsMap[permutationsSize];

    for (int i = 0; i < permutationsSize; i++)
        permutationsMap[i] = i & permutationsMask;

    for (int i = 0; i < permutationsSize; i++)
    {
        int swapIndex = generator() % permutationsSize;
        swap(permutationsMap[i], permutationsMap[swapIndex]);
    }

    for (int i = 0; i < SAMPLES_COUNT; i++)
    {
        float angle = random();
        m_noiseValues[i] = vec4(cosf(angle), sinf(angle),
            (float)permutationsMap[i], (float)permutationsMap[i & permutationsMask]);
    }
}

Texture* PerlinNoise::RenderNoise(NoiseParameters noiseParameters)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    Shader*        noiseShader   = shaderManager->GetPerlinNoiseShader();
    Texture*       noiseTexture  = new Texture(noiseParameters.TextureSize,
                                               noiseParameters.TextureSize,
                                               Texture::Format::R32F,
                                               Texture::Format::RED,
                                               Texture::Filter::Linear);

    noiseShader->Use();

    noiseShader->SetImage2D("ImgOutput",        noiseTexture, 0, Texture::Format::R32F);

    noiseShader->SetUniformBlockBinding("NoiseValues", 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_noiseValuesBuffer);

    noiseShader->SetFloat("NoiseFrequency",     noiseParameters.Frequency);
    noiseShader->SetInt("OctavesAdd",           noiseParameters.OctavesCount);
                                                  
    noiseShader->SetFloat("FudgeFactor",        noiseParameters.FudgeFactor);
    noiseShader->SetFloat("Exponent",           noiseParameters.Exponent);
                                                  
    noiseShader->SetVec2("OctaveOffset",        OCTAVE_OFFSET);
                                                  
    noiseShader->SetVec2("StartPosition",       noiseParameters.StartPosition);
    noiseShader->SetVec2("FinalPosition",       noiseParameters.EndPosition);

    glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
                      Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT), 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return noiseTexture;
}

void PerlinNoise::CreateValuesBuffer()
{
    glGenBuffers(1, &m_noiseValuesBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_noiseValuesBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(m_noiseValues), &m_noiseValues, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PerlinNoise::FreeValuesBuffer()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glDeleteBuffers(1, &m_noiseValuesBuffer);
}