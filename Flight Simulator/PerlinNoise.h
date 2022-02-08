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

           const glm::vec2 OCTAVE_OFFSET = glm::vec2(1.0f, 1.0f);
                                                       
           const float     DEFAULT_FREQUENCY           = 0.025f;
                                                       
           const float     BIOMES_DEFAULT_FREQUENCY    = 0.1f;
                                                       
           const float     FUDGE_FACTOR                = 1.2f;
           const float     EXPONENT                    = 4.0f;
                                                       
           const float     BIOMES_FUDGE_FACTOR         = 1.0f;
           const float     BIOMES_EXPONENT             = 1.0f;

           const int       TEXTURE_WIDTH               = 1024;
           const int       TEXTURE_HEIGHT              = TEXTURE_WIDTH;
           
           const int       OCTAVES_COUNT               = 20;
           const int       BIOMES_OCTAVES_COUNT        = 10;

           const int       QUAD_INDICES_COUNT          = 6;
           
           const int       COMPUTE_SHADER_BLOCKS_COUNT = 8;
           const int       MAX_MIN_MAX_SHADER_STEPS    = 4;

    static const int       SAMPLES_COUNT               = 1 << 8;

public:

    typedef std::pair<float, float> MinMax;
    typedef std::pair<Texture*, MinMax**> NoiseData;

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    NoiseData RenderNoise(glm::vec2, glm::vec2, int);
    
private:

    void     GenerateNoiseValues(int seed);
             
    void     CreateValuesBuffer();
    void     FreeValuesBuffer();

    uint32_t GetComputeShaderGroupsCount(const uint32_t, const uint32_t);
    
private:

    glm::vec4      m_noiseValues[SAMPLES_COUNT]; // (x, y) components are for the vectors
                                                 // (z, w) are for the permutations map.

    Shader*        m_noiseShader;
    Shader*        m_minMaxShader;

    unsigned int   m_noiseValuesBuffer;
};