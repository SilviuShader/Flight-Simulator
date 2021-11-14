#include <functional>
#include <cmath>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"
#include "glad/glad.h"

using namespace std;
using namespace glm;

PerlinNoise::PerlinNoise(int seed)
{
    GenerateNoiseValues(seed);
    CreateValuesBuffer();
    CreateQuadBuffers();

    m_noiseShader = new Shader("Shaders/Noise.vert", "Shaders/Noise.frag");
}

PerlinNoise::~PerlinNoise()
{
    if (m_noiseShader)
    {
        delete m_noiseShader;
        m_noiseShader = nullptr;
    }

    FreeQuadBuffers();
    FreeValuesBuffer();
}

void PerlinNoise::GenerateNoiseValues(int seed)
{
    mt19937 generator(seed);
    uniform_real_distribution<float> distribution(0.0f, 2.0f * pi<float>());
    auto random = bind(distribution, generator);

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

void PerlinNoise::CreateQuadBuffers()
{
    int verticesCount = 4;

    Vertex vertices[] =
    {
        vec3(1.0f,  1.0f, 0.0f), vec2(1.0f, 1.0f),
        vec3(1.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f),
        vec3(-1.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f),
        vec3(-1.0f,  1.0f, 0.0f), vec2(0.0f, 1.0f)
    };

    unsigned int indices[] =
    {
        0, 3, 1,
        1, 3, 2
    };

    glGenVertexArrays(1, &m_quadVao);

    glBindVertexArray(m_quadVao);

    glGenBuffers(1, &m_quadVbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_quadEbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * QUAD_INDICES_COUNT, indices, GL_STATIC_DRAW);
}

void PerlinNoise::FreeQuadBuffers()
{
    glBindVertexArray(m_quadVao);

    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_quadVbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_quadEbo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_quadVao);
}

RenderTexture* PerlinNoise::RenderNoise(vec2 startPosition, vec2 finalPosition)
{
    RenderTexture* renderTexture = new RenderTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    
    renderTexture->Begin();

    m_noiseShader->Use();

    m_noiseShader->SetBlockBinding("NoiseValues", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_noiseValuesBuffer);

    m_noiseShader->SetFloat("NoiseDefaultFrequency", DEFAULT_FREQUENCY);
    m_noiseShader->SetFloat("ColorsDefaultFrequency", COLORS_DEFAULT_FREQUENCY);

    m_noiseShader->SetVec2("StartPosition", startPosition);
    m_noiseShader->SetVec2("FinalPosition", finalPosition);

    m_noiseShader->SetInt("OctavesAdd", OCTAVES_COUNT);
    m_noiseShader->SetInt("ColorsOctavesAdd", COLORS_OCTAVES_COUNT);

    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEbo);
    glDrawElements(GL_TRIANGLES, QUAD_INDICES_COUNT, GL_UNSIGNED_INT, 0);

    return renderTexture;
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
