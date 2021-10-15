#include <random>
#include <functional>
#include <cmath>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"

using namespace std;
using namespace glm;

PerlinNoise::PerlinNoise()
{
    mt19937 generator(0);
    uniform_real_distribution<float> distribution(0.0f, 2.0f * pi<float>());
    auto random = bind(distribution, generator);

    for (int i = 0; i < SAMPLES_COUNT; i++)
    {
        float angle = random();
        m_samples[i] = vec2(cosf(angle), sinf(angle));
    }

    int permutationsMapSize = SAMPLES_COUNT << 1;
    int mask = SAMPLES_COUNT - 1;

    for (int i = 0; i < permutationsMapSize; i++)
        m_permutationsMap[i] = i & mask;

#ifdef _DEBUG
    DebugNoise();
#endif
}

float PerlinNoise::GetValue(vec2 position)
{
    int mask = SAMPLES_COUNT - 1;
        
    int left = ((int)(floor(position.x))) & mask;
    int bottom = ((int)(floor(position.y))) & mask;
    int right = (left + 1) & mask;
    int top = (bottom + 1) & mask;

    float dx = position.x - (int)(floor(position.x));
    float dy = position.y - (int)(floor(position.y));

    vec2 bottomLeft = m_samples[m_permutationsMap[m_permutationsMap[left] + bottom]];
    vec2 bottomRight = m_samples[m_permutationsMap[m_permutationsMap[right] + bottom]];
    vec2 topLeft = m_samples[m_permutationsMap[m_permutationsMap[left] + top]];
    vec2 topRight = m_samples[m_permutationsMap[m_permutationsMap[right] + top]];

    vec2 toBl = vec2(dx, dy);
    vec2 toBr = vec2(toBl.x - 1.0f, dy);
    vec2 toTl = vec2(dx, toBl.y - 1.0f);
    vec2 toTr = vec2(toBl.x - 1.0f, toBl.y - 1.0f);

    float dotBl = dot(bottomLeft, toBl);
    float dotBr = dot(bottomRight, toBr);
    float dotTl = dot(topLeft, toTl);
    float dotTr = dot(topRight, toTr);

    float bottomVal = Lerp(dotBl, dotBr, Smoothstep(dx));
    float topVal = Lerp(dotTl, dotTr, Smoothstep(dx));
    
    float result = Lerp(bottomVal, topVal, Smoothstep(dy));

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

void PerlinNoise::DebugNoise()
{
    ofstream ofs;
    ofs.open("./noise.ppm", ios::out | ios::binary);
    int imageWidth = 1024;
    int imageHeight = 1024;
    ofs << "P6\n" << imageWidth << " " << imageHeight << "\n255\n";
    for (unsigned k = 0; k < imageWidth * imageHeight; ++k)
    {
        int py, px;
        py = k / imageWidth;
        px = k % imageWidth;
        unsigned char n = static_cast<unsigned char>(GetValue(vec2(px * 0.01f, py * 0.01f)) * 255);
        ofs << n << n << n;
    }
    ofs.close();
}

float PerlinNoise::Smoothstep(float x) const
{
    return x * x * (3.0f - 2.0f * x);
}

float PerlinNoise::Lerp(float a, float b, float value) const
{
    return a + ((b - a) * value);
}
