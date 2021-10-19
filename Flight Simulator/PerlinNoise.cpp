#include <functional>
#include <cmath>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"
#include "glad/glad.h"

using namespace std;
using namespace glm;

void PerlinNoise::NoiseValues::GenerateSamples(std::mt19937& generator)
{
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
        m_samples[i] = vec4(cosf(angle), sinf(angle),
           (float)permutationsMap[i], (float)permutationsMap[i & permutationsMask]);
    }
}

vec2 PerlinNoise::NoiseValues::GetSample(int index) const
{
    if (index < 0 || index >= SAMPLES_COUNT)
    {
        cout << "PERLIN::NOISE::INDEX_OUT_OF_BOUNDS\n" << endl;
        return vec2(0.0f, 0.0f);
    }

    return vec2(m_samples[index].x, m_samples[index].y);
}

int PerlinNoise::NoiseValues::GetPermutation(int index) const
{
    int permutationsCount = SAMPLES_COUNT << 1;
    if (index < 0 || index >= permutationsCount)
    {
        cout << "PERLIN::NOISE::INDEX_OUT_OF_BOUNDS\n" << endl;
        return 0;
    }

    if (index < SAMPLES_COUNT)
        return (int)m_samples[index].z;

    return (int)m_samples[index - SAMPLES_COUNT].w;
}

PerlinNoise::PerlinNoise(int seed)
{
    mt19937 generator(seed);

    m_noiseValues.GenerateSamples(generator);

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

int PerlinNoise::HashPermutationsMap(int val1, int val2)
{
    return m_noiseValues.GetPermutation(m_noiseValues.GetPermutation(val1) + val2);
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

    glBindVertexArray(m_quadVao);

    m_noiseShader->Use();

    m_noiseShader->SetBlockBinding("NoiseValues", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_noiseValuesBuffer);

    m_noiseShader->SetFloat("NoiseDefaultFrequency", DEFAULT_FREQUENCY);

    m_noiseShader->SetVec2("StartPosition", startPosition);
    m_noiseShader->SetVec2("FinalPosition", finalPosition);

    m_noiseShader->SetInt("OctavesAdd", OCTAVES_COUNT);

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
