#version 430 core

#define NOISE_SAMPLES_COUNT     256
#define NOISE_DEFAULT_FREQUENCY 0.1
#define TERRAIN_AMPLITUDE       10.0
#define DETAIL_OCTAVES_COUNT    4

layout (triangles, equal_spacing, ccw) in;

layout (std140, binding=2) uniform NoiseValues
{
    vec4 Samples[NOISE_SAMPLES_COUNT];
};

uniform mat4 View;
uniform mat4 Projection;

in vec3 TESInputPosition[];
in vec3 TESInputRawPosition[];
in vec3 TESInputColor[];

out vec3 FSInputColor;

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

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

float getNoiseValue(vec2 position)
{
    int mask = NOISE_SAMPLES_COUNT - 1;

    position *= NOISE_DEFAULT_FREQUENCY;

    int left   = (int(floor(position.x))) & mask;
    int bottom = (int(floor(position.y))) & mask;
    int right  =                 (left + 1) & mask;
    int top    =               (bottom + 1) & mask;

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

float getCombinedNoiseValue(vec2 position, int startOctave)
{
    float frequency = float(1 << startOctave);
    float amplitude = 1.0 / frequency;
    float result = 0.0f;

    for (int i = 0; i < DETAIL_OCTAVES_COUNT; i++)
    {
        result += getNoiseValue(position * frequency) * amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return result;
}

void main()
{
    vec3 rawPosition   = interpolate3D(TESInputRawPosition[0], TESInputRawPosition[1], TESInputRawPosition[2]);
    float noise = getCombinedNoiseValue(rawPosition.xy, 4);
    vec3 worldPosition = interpolate3D(TESInputPosition[0], TESInputPosition[1], TESInputPosition[2]);
    worldPosition.y += noise * TERRAIN_AMPLITUDE;

    FSInputColor = interpolate3D(TESInputColor[0], TESInputColor[1], TESInputColor[2]) * float(Samples[255].z);
    gl_Position  = Projection * View * vec4(worldPosition, 1.0f);
}