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

    glDispatchCompute(GetComputeShaderGroupsCount(TEXTURE_WIDTH, COMPUTE_SHADER_BLOCKS_COUNT),
                      GetComputeShaderGroupsCount(TEXTURE_HEIGHT, COMPUTE_SHADER_BLOCKS_COUNT), 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return noiseTexture;
}

PerlinNoise::MinMax** PerlinNoise::GetMinMax(Texture* noiseTexture, int quadTreeLevels)
{
    int quadTreesDivisionsCount = 1 << (quadTreeLevels - 1);
    int div = TEXTURE_WIDTH / quadTreesDivisionsCount;

    assert(TEXTURE_WIDTH > quadTreesDivisionsCount && quadTreesDivisionsCount > 0 &&
        "The width of the noise texture must be greater than the width of the texture containing the min and max values of the noise.");

    assert(TEXTURE_WIDTH % quadTreeLevels == 0 && ((div & (div - 1)) == 0) &&
        "Texture width has to be equal to quadTreeLevels * pow(2, k), where k > 1.");

    pair<Texture*, Texture*> currentTextures = ComputeDownscale(m_minMaxShader,  
                                                                noiseTexture, 
                                                                noiseTexture, 
                                                                quadTreesDivisionsCount, 
                                                                MAX_MIN_MAX_SHADER_STEPS);

    int                      minMaxTexWidth  = currentTextures.first->GetWidth();
    int                      minMaxTexHeight = currentTextures.first->GetHeight();
                                             
    float*                   minPixels       = new float[minMaxTexWidth * minMaxTexHeight];
    float*                   maxPixels       = new float[minMaxTexWidth * minMaxTexHeight];

    MinMax**                 minMaxValues    = new MinMax * [minMaxTexWidth];
    
    for (int i = 0; i < minMaxTexWidth; i++)
        minMaxValues[i] = new MinMax[minMaxTexHeight];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextures.first->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, minPixels);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextures.second->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, maxPixels);

    for (int width = 0; width < minMaxTexWidth; width++)
        for (int height = 0; height < minMaxTexHeight; height++)
            minMaxValues[width][height] = make_pair(minPixels[height * minMaxTexWidth + width],
                                                    maxPixels[height * minMaxTexWidth + width]);

    if (maxPixels)
    {
        delete[] maxPixels;
        maxPixels = nullptr;
    }

    if (minPixels)
    {
        delete[] minPixels;
        minPixels = nullptr;
    }

    if (currentTextures.second && currentTextures.second != noiseTexture)
    {
        delete currentTextures.second;
        currentTextures.second = nullptr;
    }

    if (currentTextures.first && currentTextures.first != noiseTexture)
    {
        delete currentTextures.first;
        currentTextures.first = nullptr;
    }

    return minMaxValues;
}

PerlinNoise::HeightBiome** PerlinNoise::GetHeightBiome(Texture* heightTexture, Texture* biomesTexture, int heightBiomesLevels)
{
    int heightBiomesDivisionsCount = 1 << (heightBiomesLevels - 1);
    int div2                       = TEXTURE_WIDTH / heightBiomesDivisionsCount;

    assert(TEXTURE_WIDTH > heightBiomesDivisionsCount && heightBiomesDivisionsCount > 0 &&
           "The width of the noise texture must be greater than the width of the texture containing the height and biome values of the noise.");

    assert(TEXTURE_WIDTH % heightBiomesLevels == 0 && ((div2 & (div2 - 1)) == 0) &&
           "Texture width has to be equal to heightBiomesLevels * pow(2, k), where k > 1.");

    pair<Texture*, Texture*> currentTextures      = ComputeDownscale(m_heightBiomeMipMapShader, 
                                                                     heightTexture, 
                                                                     biomesTexture, 
                                                                     heightBiomesDivisionsCount, 
                                                                     MAX_HEIGHT_BIOME_SHADER_STEPS);

    int                      heightBiomeTexWidth  = currentTextures.first->GetWidth();
    int                      heightBiomeTexHeight = currentTextures.first->GetHeight();

    float*                   heightPixels         = new float[heightBiomeTexWidth * heightBiomeTexHeight];
    float*                   biomePixels          = new float[heightBiomeTexWidth * heightBiomeTexHeight];

    HeightBiome**            heightBiomeValues    = new HeightBiome * [heightBiomeTexWidth];

    for (int i = 0; i < heightBiomeTexWidth; i++)
        heightBiomeValues[i] = new HeightBiome[heightBiomeTexHeight];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextures.first->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, heightPixels);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextures.second->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, biomePixels);

    for (int width = 0; width < heightBiomeTexWidth; width++)
        for (int height = 0; height < heightBiomeTexHeight; height++)
            heightBiomeValues[width][height] = make_pair(heightPixels[height * heightBiomeTexWidth + width],
                                                         biomePixels [height * heightBiomeTexWidth + width]);

    if (biomePixels)
    {
        delete[] biomePixels;
        biomePixels = nullptr;
    }

    if (heightPixels)
    {
        delete[] heightPixels;
        heightPixels = nullptr;
    }

    if (currentTextures.second && currentTextures.second != biomesTexture)
    {
        delete currentTextures.second;
        currentTextures.second = nullptr;
    }

    if (currentTextures.first && currentTextures.first != heightTexture)
    {
        delete currentTextures.first;
        currentTextures.first = nullptr;
    }

    return heightBiomeValues;
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

pair<Texture*, Texture*> PerlinNoise::ComputeDownscale(Shader* downscaleShader, Texture* noiseTexture1, Texture* noiseTexture2, int divisionsCount, int shaderSteps)
{
    Texture* currentTexture1 = noiseTexture1;
    Texture* currentTexture2 = noiseTexture2;

    Texture* newTexture1     = nullptr;
    Texture* newTexture2     = nullptr;

    int itersCount = std::log2(TEXTURE_WIDTH / divisionsCount);

    int add = 0;
    int iter = add;
    do
    {
        add = (iter + shaderSteps > itersCount) ? itersCount - iter : shaderSteps;
        iter += add;

        if (!newTexture1)
            newTexture1 = new Texture(TEXTURE_WIDTH >> iter,
                                      TEXTURE_WIDTH >> iter,
                                      Texture::Format::R32F,
                                      Texture::Format::RED,
                                      Texture::Filter::Linear);
        
        if (!newTexture2)
            newTexture2 = new Texture(TEXTURE_WIDTH >> iter,
                                      TEXTURE_WIDTH >> iter,
                                      Texture::Format::R32F,
                                      Texture::Format::RED,
                                      Texture::Filter::Linear);
        

        downscaleShader->Use();

        downscaleShader->SetImage2D("ImageInput1",  currentTexture1, 0, Texture::Format::R32F);
        downscaleShader->SetImage2D("ImageInput2",  currentTexture2, 1, Texture::Format::R32F);

        downscaleShader->SetImage2D("ImageOutput1", newTexture1,     2, Texture::Format::R32F);
        downscaleShader->SetImage2D("ImageOutput2", newTexture2,     3, Texture::Format::R32F);

        downscaleShader->SetInt("Iterations",         add);
        
        glDispatchCompute(GetComputeShaderGroupsCount(currentTexture1->GetWidth()  >> 1, COMPUTE_SHADER_BLOCKS_COUNT),
                          GetComputeShaderGroupsCount(currentTexture1->GetHeight() >> 1, COMPUTE_SHADER_BLOCKS_COUNT), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        /*
        float* minPixels = new float[newTexture1->GetWidth() * newTexture1->GetHeight()];
        float* maxPixels = new float[newTexture2->GetWidth() * newTexture2->GetHeight()];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, newTexture1->GetTextureID());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, minPixels);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, newTexture2->GetTextureID());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, maxPixels);
        */

        if (currentTexture2 && currentTexture2 != noiseTexture2)
        {
            delete currentTexture2;
            currentTexture2 = nullptr;
        }

        if (currentTexture1 && currentTexture1 != noiseTexture1)
        {
            delete currentTexture1;
            currentTexture1 = nullptr;
        }

        currentTexture1 = newTexture1;
        newTexture1 = nullptr;

        currentTexture2 = newTexture2;
        newTexture2 = nullptr;
    } while (iter < itersCount);

    return make_pair(currentTexture1, currentTexture2);
}

uint32_t PerlinNoise::GetComputeShaderGroupsCount(const uint32_t size, const uint32_t numBlocks)
{
    return (size + numBlocks - 1) / numBlocks;
}
