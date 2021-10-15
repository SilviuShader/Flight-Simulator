#include <functional>
#include <cmath>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.h"

using namespace std;
using namespace glm;

PerlinNoise::PerlinNoise(int seed)
{
    mt19937 generator(seed);

    uniform_real_distribution<float> distribution(0.0f, 2.0f * pi<float>());
    auto random = bind(distribution, generator);

    for (int i = 0; i < SAMPLES_COUNT; i++)
    {
        float angle = random();
        m_samples[i] = vec2(cosf(angle), sinf(angle));
    }

    CreatePermutationsMap(generator);

#ifdef _DEBUG
    DebugNoise();
#endif
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

    vec2 bottomLeft  = m_samples[HashPermutationsMap(left,  bottom)];
    vec2 bottomRight = m_samples[HashPermutationsMap(right, bottom)];
    vec2 topLeft     = m_samples[HashPermutationsMap(left,  top)];
    vec2 topRight    = m_samples[HashPermutationsMap(right, top)];

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

int PerlinNoise::HashPermutationsMap(int val1, int val2)
{
    return m_permutationsMap[m_permutationsMap[val1] + val2];
}

void PerlinNoise::CreatePermutationsMap(mt19937& generator)
{
    int permutationsMapSize = SAMPLES_COUNT << 1;
    int mask                = SAMPLES_COUNT - 1;

    for (int i = 0; i < permutationsMapSize; i++)
        m_permutationsMap[i] = i & mask;

    for (int i = 0; i < permutationsMapSize; i++)
    {
        int swapIndex = generator() % permutationsMapSize;
        swap(m_permutationsMap[i], m_permutationsMap[swapIndex]);
    }
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

float PerlinNoise::Smoothstep(float x) const
{
    return x * x * (3.0f - 2.0f * x);
}

float PerlinNoise::Lerp(float a, float b, float value) const
{
    return a + ((b - a) * value);
}
