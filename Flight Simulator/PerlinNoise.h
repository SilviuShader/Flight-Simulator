#pragma once

#include <random>
#include <glm/glm.hpp>

class PerlinNoise
{
private:

           const float DEFAULT_FREQUENCY     = 0.1f;
           const int   OCTAVES_COUNT         = 4;

           const int   DEBUG_IMAGE_WIDTH     = 1024;
           const int   DEBUG_IMAGE_HEIGHT    = 1024;

           const float DEBUG_IMAGE_FREQUENCY = 0.01f;

    static const int   SAMPLES_COUNT         = 1 << 8; // must be a power of 2.

private:

    struct NoiseValues
    {
        glm::vec4 Samples[SAMPLES_COUNT]; // (x, y) components are for the vectors
                                          // (z, w) are for the permutations map.

    public:
       
        int  GetPermutation(int) const;
    };

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    float        GetValue(glm::vec2);
    float        GetCombinedValue(glm::vec2);

    unsigned int GetNoiseValuesBuffer() const;

private:

    int   HashPermutationsMap(int, int);

    void  DebugNoise();

    void  CreateBuffer();
    void  FreeBuffer();

    float Smoothstep(float)         const;
    float Lerp(float, float, float) const;
    
private:

    NoiseValues  m_noiseValues;

    unsigned int m_noiseValuesBuffer;
};