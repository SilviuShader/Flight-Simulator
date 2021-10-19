#pragma once

#include <random>
#include <glm/glm.hpp>

#include "RenderTexture.h"
#include "Shader.h"

class PerlinNoise
{
private:

           const int   TEXTURE_WIDTH         = 1024;
           const int   TEXTURE_HEIGHT        = 1024;

           const float DEBUG_IMAGE_FREQUENCY = 0.01f;

           const float DEFAULT_FREQUENCY     = 0.025f;
           const int   OCTAVES_COUNT         = 10;

           const int   QUAD_INDICES_COUNT    = 6;

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

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoords;
    };

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    RenderTexture* RenderNoise(glm::vec2, glm::vec2);
    
private:

    int  HashPermutationsMap(int, int);

    void CreateQuadBuffers();
    void FreeQuadBuffers();

    void CreateValuesBuffer();
    void FreeValuesBuffer();
    
private:

    NoiseValues    m_noiseValues;
    Shader*        m_noiseShader;

    unsigned int   m_noiseValuesBuffer;

    unsigned int   m_quadVao;
    unsigned int   m_quadVbo;
    unsigned int   m_quadEbo;
};