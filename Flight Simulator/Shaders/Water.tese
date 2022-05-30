#version 430 core

layout (triangles, fractional_odd_spacing, ccw) in;

in vec2 TESInputTexCoords[];
in vec4 TESInputWorldPosition[];
in vec4 TESInputReflectionPosition[];

uniform mediump mat4 View;
uniform mediump mat4 Projection;

uniform float Time;
uniform float ScreenEdgeCorrectionDistance;

uniform vec4 WavesWeights;
uniform vec4 WavesSpeeds;
uniform vec4 WavesOffsets;
uniform vec4 WavesRadiuses;

uniform vec2 WaveADirection;
uniform vec2 WaveBDirection;
uniform vec2 WaveCDirection;
uniform vec2 WaveDDirection;

out vec2 GSInputTexCoords;
out vec4 GSInputWorldPosition;
out vec4 GSInputReflectionPosition;
out vec4 GSInputRealClipCoords;

vec2 interpolate2D(vec2 u, vec2 v, vec2 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 interpolate3D(vec3 u, vec3 v, vec3 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec4 interpolate4D(vec4 u, vec4 v, vec4 w)
{
    return u * gl_TessCoord.x + v * gl_TessCoord.y + w * gl_TessCoord.z;
}

vec3 trochoidalWave(vec2 rotateDirection, vec2 xz, float speed, float offset, float radius)
{
    float spaceProjection = dot(rotateDirection, xz);
    vec3 result;

    float horizontalMovement = sin(Time * speed + spaceProjection * offset) * radius;
    result.x = horizontalMovement * rotateDirection.x;
    result.z = horizontalMovement * rotateDirection.y;

    result.y = cos(Time * speed + spaceProjection * offset) * radius;

    return result;
}

vec3 vertexDisplacement(vec2 xz)
{
    vec4 normalizedWeights = WavesWeights / dot(WavesWeights, vec4(1.0, 1.0, 1.0, 1.0));

    vec3 waveA = trochoidalWave(WaveADirection, xz, WavesSpeeds.x, WavesOffsets.x, WavesRadiuses.x);
    vec3 waveB = trochoidalWave(WaveBDirection, xz, WavesSpeeds.y, WavesOffsets.y, WavesRadiuses.y);
    vec3 waveC = trochoidalWave(WaveCDirection, xz, WavesSpeeds.z, WavesOffsets.z, WavesRadiuses.z);
    vec3 waveD = trochoidalWave(WaveDDirection, xz, WavesSpeeds.w, WavesOffsets.w, WavesRadiuses.w);

    return waveA * normalizedWeights.x + waveB * normalizedWeights.y + waveC * normalizedWeights.z + waveD * normalizedWeights.w;
}

void main()
{
    GSInputWorldPosition      = interpolate4D(TESInputWorldPosition[0],  TESInputWorldPosition[1],  TESInputWorldPosition[2]);
    GSInputReflectionPosition = interpolate4D(TESInputReflectionPosition[0], TESInputReflectionPosition[1], TESInputReflectionPosition[2]);

    GSInputWorldPosition.xyz += vertexDisplacement(GSInputWorldPosition.xz);

    vec4 correctReflectionPosition = Projection * View * GSInputWorldPosition;

    vec2 clipSpaceCords = GSInputReflectionPosition.xy / GSInputReflectionPosition.w * 0.5 + 0.5;
    float horizontalDistance = min(clipSpaceCords.x, 1.0 - clipSpaceCords.x);
    float verticalDistance = min(clipSpaceCords.y, 1.0 - clipSpaceCords.y);
    float minDistance = min(horizontalDistance, verticalDistance);

    float correctnessPercentage = clamp(minDistance / ScreenEdgeCorrectionDistance, 0.0, 1.0);

    GSInputReflectionPosition = mix(correctReflectionPosition, GSInputReflectionPosition, correctnessPercentage);


    GSInputTexCoords          = interpolate2D(TESInputTexCoords[0],          TESInputTexCoords[1],          TESInputTexCoords[2]);

    GSInputRealClipCoords     = Projection * View * GSInputWorldPosition;

    gl_Position               = GSInputRealClipCoords;
}