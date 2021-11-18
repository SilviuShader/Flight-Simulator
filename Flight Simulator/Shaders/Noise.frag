#version 430 core

#define NOISE_SAMPLES_COUNT 256

layout (std140, binding = 0) uniform NoiseValues
{
    vec4 Samples[NOISE_SAMPLES_COUNT];
};

uniform float NoiseDefaultFrequency;
uniform float ColorsDefaultFrequency;

uniform int OctavesAdd;
uniform int ColorsOctavesAdd;

uniform float FudgeFactor;
uniform float Exponent;
uniform vec2  OctaveOffset;

uniform vec2 StartPosition;
uniform vec2 FinalPosition;

in vec2 FSInputCoords;

layout(location = 0) out vec3 FSOutFragColor;

int getPermutation(int index)
{
    int permutationsCount = NOISE_SAMPLES_COUNT << 1;
    if (index < 0 || index >= permutationsCount)
    {
        return 0;
    }

    if (index < NOISE_SAMPLES_COUNT)
        return int(Samples[index].z);

    return int(Samples[index - NOISE_SAMPLES_COUNT].w);
}

int hashPermutationsMap(int val1, int val2)
{
    return getPermutation(getPermutation(val1) + val2);
}

float smoothstep(float x)
{
    return x * x * (3.0 - 2.0 * x);
}

float getNoiseValue(vec2 position, float defaultFrequency)
{
    int mask = NOISE_SAMPLES_COUNT - 1;

    position *= defaultFrequency;

    int left   = (int(floor(position.x))) & mask;
    int bottom = (int(floor(position.y))) & mask;
    int right  =               (left + 1) & mask;
    int top    =             (bottom + 1) & mask;

    float dx = position.x - int(floor(position.x));
    float dy = position.y - int(floor(position.y));

    vec2 bottomLeft  = Samples[hashPermutationsMap(left,  bottom)].xy;
    vec2 bottomRight = Samples[hashPermutationsMap(right, bottom)].xy;
    vec2 topLeft     = Samples[hashPermutationsMap(left,  top)].xy;
    vec2 topRight    = Samples[hashPermutationsMap(right, top)].xy;

    vec2 toBottomLeft  = vec2(dx,       dy);
    vec2 toBottomRight = vec2(dx - 1.0, dy);
    vec2 toTopLeft     = vec2(dx,       dy - 1.0);
    vec2 toTopRight    = vec2(dx - 1.0, dy - 1.0);

    float dotBottomLeft  = dot(bottomLeft,  toBottomLeft);
    float dotBottomRight = dot(bottomRight, toBottomRight);
    float dotTopLeft     = dot(topLeft,     toTopLeft);
    float dotTopRight    = dot(topRight,    toTopRight);

    float horizontalPercentage = smoothstep(dx);
    float verticalPercentage   = smoothstep(dy);

    float bottomValue = mix(dotBottomLeft, dotBottomRight, horizontalPercentage);
    float topValue    = mix(dotTopLeft, dotTopRight, horizontalPercentage);
    
    float result = mix(bottomValue, topValue, verticalPercentage);

    result *= 0.5;
    result += 0.5;

    return result;
}

float getCombinedNoiseValue(vec2 position, float defaultFrequency, int maxOctaves)
{
    float frequency = 1.0;
    float amplitude = 1.0;
    float result = 0.0;

    float amplitudeSum = 0.0f;

    for (int i = 0; i < maxOctaves; i++)
    {
        result += getNoiseValue(position * frequency + (OctaveOffset * i), defaultFrequency) * amplitude;
        float percentage = clamp(result, 0.0, 1.0);
        
        amplitudeSum += amplitude;

        frequency *= 2.0;
        amplitude *= 0.5;
    }

    result = result / amplitudeSum;
    result = pow(result * FudgeFactor, Exponent);

    return result;
}

void main()
{
    vec2  finalStartDiff = FinalPosition - StartPosition;
    vec2  noisePosition  = vec2(FSInputCoords.x * finalStartDiff.x, FSInputCoords.y * finalStartDiff.y) + StartPosition;
    float noise          = getCombinedNoiseValue(noisePosition, NoiseDefaultFrequency, OctavesAdd);
    float colorNoise     = getCombinedNoiseValue(noisePosition, ColorsDefaultFrequency, ColorsOctavesAdd);

    FSOutFragColor = vec3(noise, colorNoise, noise);
}