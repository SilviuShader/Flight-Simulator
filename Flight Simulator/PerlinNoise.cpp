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

Texture* PerlinNoise::RenderPerlinNoise(NoiseParameters noiseParameters)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    Shader*        noiseShader   = shaderManager->GetPerlinNoiseShader();

    return RenderNoise(noiseShader, noiseParameters, false);
}

Texture* PerlinNoise::RenderSimplexNoise(NoiseParameters noiseParameters, bool normalize)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    Shader*        noiseShader   = shaderManager->GetSimplexNoiseShader();

    return RenderNoise(noiseShader, noiseParameters, normalize);
}


Texture* PerlinNoise::RenderNoise(Shader* noiseShader, NoiseParameters noiseParameters, bool normalize)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    Texture*       noiseTexture  = new Texture(noiseParameters.TextureSize,
                                               noiseParameters.TextureSize,
                                               Texture::Format::R32F,
                                               Texture::Format::RED,
                                               Texture::Filter::Linear);

    noiseShader->Use();

    noiseShader->SetImage2D("ImgOutput",        noiseTexture, 0, Texture::Format::R32F);

    noiseShader->SetUniformBlockBinding("NoiseValues", 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_noiseValuesBuffer);

    if (noiseShader->HasUniform("PrepareNormalize"))
        noiseShader->SetInt("PrepareNormalize", 0);
    else
        normalize = false;

    unsigned int minMaxBuffer = -1;

    if (normalize)
    {
        minMaxBuffer = Texture::CreateMinMaxBuffer();

        noiseShader->SetShaderStorageBlockBinding("MinMaxValues", 2);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, minMaxBuffer);

        noiseShader->SetInt("MinMaxBufferValue", Texture::MIN_MAX_BUFFER_VALUE);
        noiseShader->SetInt("PrepareNormalize", 1);
    }

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

    if (normalize)
    {
        Shader* texture2DNormalizeShader = shaderManager->GetTexture2DNormalizeShader();

        texture2DNormalizeShader->Use();

        texture2DNormalizeShader->SetImage2D("ImgOutput", noiseTexture, 0, Texture::Format::R32F);
        texture2DNormalizeShader->SetShaderStorageBlockBinding("MinMaxValues", 1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, minMaxBuffer);

        texture2DNormalizeShader->SetInt("MinMaxBufferValue", Texture::MIN_MAX_BUFFER_VALUE);

        glDispatchCompute(Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
                          Texture::GetComputeShaderGroupsCount(noiseParameters.TextureSize, COMPUTE_SHADER_BLOCKS_COUNT),
                          1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
        Texture::FreeMinMaxBuffer(minMaxBuffer);
    }

    return noiseTexture;
}

void PerlinNoise::CreateValuesBuffer()
{
    glGenBuffers(1, &m_noiseValuesBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_noiseValuesBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(m_noiseValues), m_noiseValues, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PerlinNoise::FreeValuesBuffer()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glDeleteBuffers(1, &m_noiseValuesBuffer);
}