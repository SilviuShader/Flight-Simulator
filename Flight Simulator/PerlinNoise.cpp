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
    m_minMaxShader            = new Shader("Shaders/MinMax.comp");
    m_heightBiomeMipMapShader = new Shader("Shaders/HeightBiomeMipMap.comp");
}

PerlinNoise::~PerlinNoise()
{
    if (m_heightBiomeMipMapShader)
    {
        delete m_heightBiomeMipMapShader;
        m_heightBiomeMipMapShader = nullptr;
    }

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

PerlinNoise::NoiseData PerlinNoise::RenderNoise(vec2 startPosition, vec2 finalPosition, int quadTreeLevels, int heightBiomesLevels)
{
    assert(TEXTURE_WIDTH == TEXTURE_HEIGHT && 
           "The noise texture must have the width equal to the height.");

    int quadTreesDivisionsCount = 1 << (quadTreeLevels - 1);
    int div = TEXTURE_WIDTH / quadTreesDivisionsCount;

    assert(TEXTURE_WIDTH > quadTreesDivisionsCount && quadTreesDivisionsCount > 0 &&
           "The width of the noise texture must be greater than the width of the texture containing the min and max values of the noise.");
    
    assert(TEXTURE_WIDTH % quadTreeLevels == 0 && ((div & (div - 1)) == 0) &&
           "Texture width has to be equal to quadTreeLevels * pow(2, k), where k > 1.");

    int heightBiomesDivisionsCount = 1 << (heightBiomesLevels - 1);
    int div2                       = TEXTURE_WIDTH / heightBiomesDivisionsCount;

    assert(TEXTURE_WIDTH > heightBiomesDivisionsCount && heightBiomesDivisionsCount > 0 &&
        "The width of the noise texture must be greater than the width of the texture containing the height and biome values of the noise.");

    assert(TEXTURE_WIDTH % heightBiomesLevels == 0 && ((div2 & (div2 - 1)) == 0) &&
        "Texture width has to be equal to heightBiomesLevels * pow(2, k), where k > 1.");

    Texture* noiseTexture = new Texture(TEXTURE_WIDTH, 
                                        TEXTURE_HEIGHT, 
                                        Texture::Format::RGBA32F, 
                                        Texture::Format::RGBA,
                                        Texture::Filter::Linear);

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

    glDispatchCompute(GetComputeShaderGroupsCount(TEXTURE_WIDTH,  COMPUTE_SHADER_BLOCKS_COUNT), 
                      GetComputeShaderGroupsCount(TEXTURE_HEIGHT, COMPUTE_SHADER_BLOCKS_COUNT), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    Texture* currentTexture = ComputeDownscale(m_minMaxShader, noiseTexture, quadTreesDivisionsCount, MAX_MIN_MAX_SHADER_STEPS);

    int minMaxTexWidth  = currentTexture->GetWidth();
    int minMaxTexHeight = currentTexture->GetHeight();

    float* minMaxPixels = new float[minMaxTexWidth * minMaxTexHeight * 4];

    MinMax** minMaxValues = new MinMax*[minMaxTexWidth];
    for (int i = 0; i < minMaxTexWidth; i++)
        minMaxValues[i] = new MinMax[minMaxTexHeight];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, minMaxPixels);

    for (int width = 0; width < minMaxTexWidth; width++)
        for (int height = 0; height < minMaxTexHeight; height++)
            minMaxValues[width][height] = make_pair(minMaxPixels[(height * minMaxTexWidth + width) * 4],
                                                    minMaxPixels[(height * minMaxTexWidth + width) * 4 + 1]);
    
    if (minMaxPixels)
    {
        delete[] minMaxPixels;
        minMaxPixels = nullptr;
    }

    if (currentTexture && currentTexture != noiseTexture)
    {
        delete currentTexture;
        currentTexture = nullptr;
    }

    currentTexture = ComputeDownscale(m_heightBiomeMipMapShader, noiseTexture, heightBiomesDivisionsCount, MAX_HEIGHT_BIOME_SHADER_STEPS);

    int heightBiomeTexWidth  = currentTexture->GetWidth();
    int heightBiomeTexHeight = currentTexture->GetHeight();

    float* heightBiomePixels = new float[heightBiomeTexWidth * heightBiomeTexHeight * 4];

    HeightBiome** heightBiomeValues = new HeightBiome*[heightBiomeTexWidth];
    for (int i = 0; i < heightBiomeTexWidth; i++)
        heightBiomeValues[i] = new HeightBiome[heightBiomeTexHeight];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, heightBiomePixels);

    for (int width = 0; width < heightBiomeTexWidth; width++)
        for (int height = 0; height < heightBiomeTexHeight; height++)
            heightBiomeValues[width][height] = make_pair(heightBiomePixels[(height * heightBiomeTexWidth + width) * 4],
                                                         heightBiomePixels[(height * heightBiomeTexWidth + width) * 4 + 1]);

    if (heightBiomePixels)
    {
        delete[] heightBiomePixels;
        heightBiomePixels = nullptr;
    }

    if (currentTexture && currentTexture != noiseTexture)
    {
        delete currentTexture;
        currentTexture = nullptr;
    }

    NoiseData result;
    result.NoiseTexture = noiseTexture;
    result.MinMax       = minMaxValues;
    result.HeightBiome  = heightBiomeValues;

    return result;
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

Texture* PerlinNoise::ComputeDownscale(Shader* downscaleShader, Texture* noiseTexture, int divisionsCount, int shaderSteps)
{
    Texture* currentTexture = noiseTexture;
    Texture* newTexture = nullptr;

    int itersCount = std::log2(TEXTURE_WIDTH / divisionsCount);

    int add = 0;
    int iter = add;
    bool initial = true;
    do
    {
        add = (iter + shaderSteps > itersCount) ? itersCount - iter : shaderSteps;
        iter += add;

        if (!newTexture)
        {
            newTexture = new Texture(TEXTURE_WIDTH >> iter,
                TEXTURE_WIDTH >> iter,
                Texture::Format::RGBA32F,
                Texture::Format::RGBA,
                Texture::Filter::Linear);
        }
        downscaleShader->Use();

        downscaleShader->SetImage2D("ImgInput",  currentTexture, 0);
        downscaleShader->SetImage2D("ImgOutput", newTexture,     1);

        downscaleShader->SetInt("Iterations",    add);
        downscaleShader->SetInt("InitialPhase",  ((initial) ? 1 : 0));

        glDispatchCompute(GetComputeShaderGroupsCount(currentTexture->GetWidth()  >> 1, COMPUTE_SHADER_BLOCKS_COUNT),
                          GetComputeShaderGroupsCount(currentTexture->GetHeight() >> 1, COMPUTE_SHADER_BLOCKS_COUNT), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        if (currentTexture && currentTexture != noiseTexture)
        {
            delete currentTexture;
            currentTexture = nullptr;
        }

        currentTexture = newTexture;
        newTexture = nullptr;

        initial = false;
    } while (iter < itersCount);

    return currentTexture;
}

uint32_t PerlinNoise::GetComputeShaderGroupsCount(const uint32_t size, const uint32_t numBlocks)
{
    return (size + numBlocks - 1) / numBlocks;
}
