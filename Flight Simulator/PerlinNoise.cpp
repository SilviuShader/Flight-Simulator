#include <functional>
#include <cmath>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"
#include "glad/glad.h"
#include "VertexTypes.h"

using namespace std;
using namespace glm;

PerlinNoise::PerlinNoise(int seed)
{
    GenerateNoiseValues(seed);
    CreateValuesBuffer();

    m_noiseShader             = new Shader("Shaders/Noise.comp");
}

PerlinNoise::~PerlinNoise()
{    
    if (m_noiseShader)
    {
        delete m_noiseShader;
        m_noiseShader = nullptr;
    }

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

Texture* PerlinNoise::RenderNoise(vec2 startPosition, vec2 finalPosition, NoiseParameters noiseParameters)
{
    assert(TEXTURE_WIDTH == TEXTURE_HEIGHT &&
           "The noise texture must have the width equal to the height.");

    Texture* noiseTexture = new Texture(TEXTURE_WIDTH,
                                        TEXTURE_HEIGHT,
                                        Texture::Format::R32F,
                                        Texture::Format::RED,
                                        Texture::Filter::Linear);

    m_noiseShader->Use();

    m_noiseShader->SetImage2D("ImgOutput",        noiseTexture, 0, Texture::Format::R32F);

    m_noiseShader->SetBlockBinding("NoiseValues", 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_noiseValuesBuffer);

    m_noiseShader->SetFloat("NoiseFrequency",     noiseParameters.Frequency);
    m_noiseShader->SetInt("OctavesAdd",           noiseParameters.OctavesCount);
                                                  
    m_noiseShader->SetFloat("FudgeFactor",        noiseParameters.FudgeFactor);
    m_noiseShader->SetFloat("Exponent",           noiseParameters.Exponent);
                                                  
    m_noiseShader->SetVec2("OctaveOffset",        OCTAVE_OFFSET);
                                                  
    m_noiseShader->SetVec2("StartPosition",       startPosition);
    m_noiseShader->SetVec2("FinalPosition",       finalPosition);

    glDispatchCompute(Texture::GetComputeShaderGroupsCount(TEXTURE_WIDTH, COMPUTE_SHADER_BLOCKS_COUNT),
                      Texture::GetComputeShaderGroupsCount(TEXTURE_HEIGHT, COMPUTE_SHADER_BLOCKS_COUNT), 1);

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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_noiseValuesBuffer);
}