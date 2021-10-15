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

public:

    PerlinNoise(int = 0);

    float GetValue(glm::vec2);
    float GetCombinedValue(glm::vec2);

private:

    int   HashPermutationsMap(int, int);

    void  CreatePermutationsMap(std::mt19937&);
    void  DebugNoise();

    float Smoothstep(float)         const;
    float Lerp(float, float, float) const;
    
private:

    glm::vec2 m_samples[SAMPLES_COUNT];
    int       m_permutationsMap[SAMPLES_COUNT << 1];
};