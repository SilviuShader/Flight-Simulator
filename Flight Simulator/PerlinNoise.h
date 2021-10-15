#pragma once

#include <glm/glm.hpp>

class PerlinNoise
{
private:

           const int OCTAVES_COUNT = 4;
    static const int SAMPLES_COUNT = 1 << 8; // must be a power of 2.

public:

    PerlinNoise();

    float GetValue(glm::vec2);
    float GetCombinedValue(glm::vec2);

private:

    void  DebugNoise();

    float Smoothstep(float)         const;
    float Lerp(float, float, float) const;
    
private:

    glm::vec2 m_samples[SAMPLES_COUNT];
    int       m_permutationsMap[SAMPLES_COUNT << 1];
};