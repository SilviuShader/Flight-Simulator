#pragma once

#include <random>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Utils.h"
#include "RenderTexture.h"
#include "Shader.h"

class PerlinNoise
{
private:

           const glm::vec2 OCTAVE_OFFSET                 = glm::vec2(1.0f, 1.0f);
                                                       
           const int       COMPUTE_SHADER_BLOCKS_COUNT   = 8;

           const int       MAX_MIN_MAX_SHADER_STEPS      = 4;
           const int       MAX_HEIGHT_BIOME_SHADER_STEPS = 4;

    static const int       SAMPLES_COUNT                 = 1 << 8;

public:

    struct NoiseParameters
    {
        glm::vec2 StartPosition;
        glm::vec2 EndPosition;
        float     Frequency;
        float     FudgeFactor;
        float     Exponent;
        int       OctavesCount;
        int       TextureSize;
    };

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    Texture* RenderPerlinNoise(NoiseParameters);
    Texture* RenderSimplexNoise(NoiseParameters, bool = false);

private:

    Texture* RenderNoise(Shader*, NoiseParameters, bool);

    void                          GenerateNoiseValues(int seed);
                                  
    void                          CreateValuesBuffer();
    void                          FreeValuesBuffer();
    
private:

    glm::vec4      m_noiseValues[SAMPLES_COUNT]; // (x, y) components are for the vectors
                                                 // (z, w) are for the permutations map.

    unsigned int   m_noiseValuesBuffer;
};