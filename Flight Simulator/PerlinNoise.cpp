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

    m_noiseShader = new Shader("Shaders/Noise.comp");
    m_minMaxShader = new Shader("Shaders/MinMax.comp");
}

PerlinNoise::~PerlinNoise()
{
    if (m_minMaxShader)
    {
        delete m_minMaxShader;
        m_minMaxShader = nullptr;
    }
    
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

pair<Texture*, map<pair<int, int>, pair<float, float>>> PerlinNoise::RenderNoise(vec2 startPosition, vec2 finalPosition, int quadTreeLevels)
{
    int divisionsCount = 1 << (quadTreeLevels - 1);
    int div = TEXTURE_WIDTH / divisionsCount;

    assert(TEXTURE_WIDTH == TEXTURE_HEIGHT);
    assert(quadTreeLevels % quadTreeLevels == 0 && ((div & (div - 1)) == 0));

    Texture* noiseTexture = new Texture(TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA32F, GL_RGBA);

    m_noiseShader->Use();

    m_noiseShader->SetImage2D("ImgOutput", noiseTexture, 0);

    m_noiseShader->SetBlockBinding("NoiseValues", 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_noiseValuesBuffer);

    m_noiseShader->SetFloat("NoiseDefaultFrequency", DEFAULT_FREQUENCY);
    m_noiseShader->SetInt("OctavesAdd", OCTAVES_COUNT);

    m_noiseShader->SetFloat("BiomesDefaultFrequency", BIOMES_DEFAULT_FREQUENCY);
    m_noiseShader->SetInt("BiomesOctavesAdd", BIOMES_OCTAVES_COUNT);

    m_noiseShader->SetFloat("FudgeFactor", FUDGE_FACTOR);
    m_noiseShader->SetFloat("Exponent", EXPONENT);

    m_noiseShader->SetFloat("BiomesFudgeFactor", BIOMES_FUDGE_FACTOR);
    m_noiseShader->SetFloat("BiomesExponent", BIOMES_EXPONENT);

    m_noiseShader->SetVec2("OctaveOffset", OCTAVE_OFFSET);

    m_noiseShader->SetVec2("StartPosition", startPosition);
    m_noiseShader->SetVec2("FinalPosition", finalPosition);

    glDispatchCompute(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    map<pair<int, int>, pair<float, float>> minMaxValues;

    int currentSize = TEXTURE_WIDTH;

    Texture* currentTexture = noiseTexture;
    Texture* newTexture = nullptr;

    while (currentSize > divisionsCount)
    {
        if (!newTexture)
            newTexture = new Texture(currentSize / 2, currentSize / 2, GL_RGBA32F, GL_RGBA);

        m_minMaxShader->Use();
        
        m_minMaxShader->SetImage2D("ImgInput", currentTexture, 0);
        m_minMaxShader->SetImage2D("ImgOutput", newTexture, 1);

        m_minMaxShader->SetInt("InitialPhase", ((currentTexture == noiseTexture) ? 1 : 0));

        glDispatchCompute(currentSize / 2, currentSize / 2, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        if (currentTexture && 
            currentTexture != noiseTexture)
        {
            delete currentTexture;
            currentTexture = nullptr;
        }

        currentTexture = newTexture;
        newTexture = nullptr;
        currentSize /= 2;
    }

    float* pixels = new float[currentSize * currentSize * 4];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

    for (int height = 0; height < currentSize; height++)
    {
        for (int width = 0; width < currentSize; width++)
        {
            int i = height;
            int j = width;
            minMaxValues[make_pair(width, height)] = make_pair(pixels[(i * currentSize + j) * 4],
                pixels[(i * currentSize + j) * 4 + 1]);
        }
    }

    if (pixels)
    {
        delete[] pixels;
        pixels = nullptr;
    }

    if (currentTexture && currentTexture != noiseTexture)
    {
        delete currentTexture;
        currentTexture = nullptr;
    }

    return make_pair(noiseTexture, minMaxValues);
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
