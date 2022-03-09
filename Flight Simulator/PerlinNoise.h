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
                                                       
           const float     DEFAULT_FREQUENCY             = 0.025f;
                                                         
           const float     BIOMES_DEFAULT_FREQUENCY      = 0.01f;
                                                         
           const float     FUDGE_FACTOR                  = 1.2f;
           const float     EXPONENT                      = 4.0f;
                                                         
           const float     BIOMES_FUDGE_FACTOR           = 1.0f;
           const float     BIOMES_EXPONENT               = 1.0f;
                                                         
           const int       TEXTURE_WIDTH                 = 1024;
           const int       TEXTURE_HEIGHT                = TEXTURE_WIDTH;
                                                         
           const int       OCTAVES_COUNT                 = 20;
           const int       BIOMES_OCTAVES_COUNT          = 10;
                                                         
           const int       QUAD_INDICES_COUNT            = 6;
                                                         
           const int       COMPUTE_SHADER_BLOCKS_COUNT   = 8;

           const int       MAX_MIN_MAX_SHADER_STEPS      = 4;
           const int       MAX_HEIGHT_BIOME_SHADER_STEPS = 4;

    static const int       SAMPLES_COUNT                 = 1 << 8;

public:

    typedef std::pair<float, float> MinMax;
    typedef std::pair<float, float> HeightBiome;

    struct NoiseData
    {
    public:

        Texture*      NoiseTexture; // Texture containing info about the terrain.
                                    // R -> height of terrain (normalized [0.0, 1.0])
                                    // G -> biome info (normalized [0.0f, 1.0f])
                                    // B -> same as the RED channel, may be used for future purposes.
                                    // A -> constant 1.0, may be used for future purposes.

        MinMax**      MinMax;       // 2D array containing info about the min and max altitude values
                                    // in the original noise texture. It's a simplified array of size
                                    // (1 << (quadTreeLevels - 1)) x (1 << (quadTreeLevels - 1))

        HeightBiome** HeightBiome;  // same as the above, although this field holds info regarding the 
                                    // height and the biome.
    };

public:

    PerlinNoise(int = 0);
    ~PerlinNoise();

    NoiseData RenderNoise(glm::vec2, glm::vec2, int, int);
    
private:

    void     GenerateNoiseValues(int seed);
             
    void     CreateValuesBuffer();
    void     FreeValuesBuffer();

    Texture* ComputeDownscale(Shader*, Texture*, int, int);

    uint32_t GetComputeShaderGroupsCount(const uint32_t, const uint32_t);
    
private:

    glm::vec4      m_noiseValues[SAMPLES_COUNT]; // (x, y) components are for the vectors
                                                 // (z, w) are for the permutations map.

    Shader*        m_noiseShader;
    Shader*        m_minMaxShader;
    Shader*        m_heightBiomeMipMapShader;

    unsigned int   m_noiseValuesBuffer;
};