#pragma once

#include <random>
#include <glm/glm.hpp>

#include "RenderTexture.h"
#include "Shader.h"

class PerlinNoise
{
public:

    static const float DEFAULT_FREQUENCY;
    static const int   OCTAVES_COUNT;

private:

           const int   DEBUG_IMAGE_WIDTH     = 1024;
           const int   DEBUG_IMAGE_HEIGHT    = 1024;

           const float DEBUG_IMAGE_FREQUENCY = 0.01f;

    static const int   SAMPLES_COUNT         = 1 << 8; // must be a power of 2.

private:

    struct NoiseValues
    {
    public:

        void GenerateSamples(std::mt19937&);
       
        glm::vec2 GetSample(int)      const;
        int       GetPermutation(int) const;

    private:

        glm::vec4 m_samples[SAMPLES_COUNT]; // (x, y) components are for the vectors
                                            // (z, w) are for the permutations map.
    };

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    float        GetValue(glm::vec2);
    float        GetCombinedValue(glm::vec2);

    unsigned int GetNoiseValuesBuffer() const;
    Texture*     GetNoiseTexture() const;
    
private:

    int   HashPermutationsMap(int, int);

    void  DebugNoise();

    void  RenderNoiseTexture();

    void  CreateBuffer();
    void  FreeBuffer();

    float Smoothstep(float)         const;
    float Lerp(float, float, float) const;
    
private:

    NoiseValues    m_noiseValues;
    RenderTexture* m_renderTexture;
    Shader*        m_noiseShader;

    unsigned int   m_noiseValuesBuffer;
};