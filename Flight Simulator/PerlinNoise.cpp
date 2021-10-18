#include <functional>
#include <cmath>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"
#include "glad/glad.h"

using namespace std;
using namespace glm;

const float PerlinNoise::DEFAULT_FREQUENCY = 0.005f;
const int   PerlinNoise::OCTAVES_COUNT     = 5;

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

#ifdef _DEBUG
    DebugNoise();
#endif

    CreateBuffer();

    m_noiseShader = new Shader("Shaders/Noise.vert", "Shaders/Noise.frag");

    RenderNoiseTexture();
}

PerlinNoise::~PerlinNoise()
{
    if (m_renderTexture)
    {
        delete m_renderTexture;
        m_renderTexture = nullptr;
    }

    if (m_noiseShader)
    {
        delete m_noiseShader;
        m_noiseShader = nullptr;
    }

    FreeBuffer();
}

float PerlinNoise::GetValue(vec2 position)
{
    int mask = SAMPLES_COUNT - 1;

    position *= DEFAULT_FREQUENCY;

    int left   = ((int)(floor(position.x))) & mask;
    int bottom = ((int)(floor(position.y))) & mask;
    int right  =                 (left + 1) & mask;
    int top    =               (bottom + 1) & mask;

    float dx = position.x - (int)(floor(position.x));
    float dy = position.y - (int)(floor(position.y));

    vec2 bottomLeft  = m_noiseValues.GetSample(HashPermutationsMap(left,  bottom));
    vec2 bottomRight = m_noiseValues.GetSample(HashPermutationsMap(right, bottom));
    vec2 topLeft     = m_noiseValues.GetSample(HashPermutationsMap(left,  top));
    vec2 topRight    = m_noiseValues.GetSample(HashPermutationsMap(right, top));

    vec2 toBottomLeft  = vec2(dx,        dy);
    vec2 toBottomRight = vec2(dx - 1.0f, dy);
    vec2 toTopLeft     = vec2(dx,        dy - 1.0f);
    vec2 toTopRight    = vec2(dx - 1.0f, dy - 1.0f);

    float dotBottomLeft  = dot(bottomLeft,  toBottomLeft);
    float dotBottomRight = dot(bottomRight, toBottomRight);
    float dotTopLeft     = dot(topLeft,     toTopLeft);
    float dotTopRight    = dot(topRight,    toTopRight);

    float horizontalPercentage = Smoothstep(dx);
    float verticalPercentage   = Smoothstep(dy);

    float bottomValue = Lerp(dotBottomLeft, dotBottomRight, horizontalPercentage);
    float topValue    = Lerp(dotTopLeft, dotTopRight, horizontalPercentage);
    
    float result = Lerp(bottomValue, topValue, verticalPercentage);

    result *= 0.5f;
    result += 0.5f;

    return result;
}

float PerlinNoise::GetCombinedValue(vec2 position)
{
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float result = 0.0f;

    for (int i = 0; i < OCTAVES_COUNT; i++)
    {
        result += GetValue(position * frequency) * amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return result;
}

unsigned int PerlinNoise::GetNoiseValuesBuffer() const
{
    return m_noiseValuesBuffer;
}

int PerlinNoise::HashPermutationsMap(int val1, int val2)
{
    return m_noiseValues.GetPermutation(m_noiseValues.GetPermutation(val1) + val2);
}

void PerlinNoise::DebugNoise()
{
    ofstream ofs;
    ofs.open("./noise.ppm", ios::out | ios::binary);

    ofs << "P6\n" << DEBUG_IMAGE_WIDTH << " " << DEBUG_IMAGE_HEIGHT << "\n255\n";

    int debugImageSize = DEBUG_IMAGE_WIDTH * DEBUG_IMAGE_HEIGHT;

    for (int i = 0; i < debugImageSize; i++)
    {
        int py, px;
        py = i / DEBUG_IMAGE_WIDTH;
        px = i % DEBUG_IMAGE_WIDTH;
        unsigned char n = static_cast<unsigned char>(GetValue(vec2(px, py) * DEBUG_IMAGE_FREQUENCY) * 255);
        ofs << n << n << n;
    }
    ofs.close();
}

void PerlinNoise::CreateBuffer()
{
    glGenBuffers(1, &m_noiseValuesBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_noiseValuesBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(m_noiseValues), &m_noiseValues, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Texture* PerlinNoise::GetNoiseTexture() const
{
    return m_renderTexture->GetTexture();
}

void PerlinNoise::FreeBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_noiseValuesBuffer);
}

float PerlinNoise::Smoothstep(float x) const
{
    return x * x * (3.0f - 2.0f * x);
}

float PerlinNoise::Lerp(float a, float b, float value) const
{
    return a + ((b - a) * value);
}

void PerlinNoise::RenderNoiseTexture()
{
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    float vertices[] = 
    {
         1.0f,  1.0f, 0.0f,             1024.0f, 1024.0f,
         1.0f, -1.0f, 0.0f,             1024.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,             0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,             0.0f, 1024.0f
    };

    unsigned int indices[] = 
    {
        0, 1, 3,
        1, 2, 3 
    };

    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW);

    m_renderTexture = new RenderTexture(SAMPLES_COUNT << 2, SAMPLES_COUNT << 2);
    m_renderTexture->Begin();

    m_noiseShader->Use();

    m_noiseShader->SetBlockBinding("NoiseValues", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, GetNoiseValuesBuffer());

    m_noiseShader->SetFloat("NoiseDefaultFrequency", DEFAULT_FREQUENCY);
    m_noiseShader->SetFloat("TerrainAmplitude", 1.0f);

    m_noiseShader->SetInt("OctavesAdd", 6);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(vao);

    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
}
