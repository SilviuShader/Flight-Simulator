#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"
#include "Light.h"

class Terrain
{
private:

    struct Vertex 
    {
    public:

        Vertex();
        Vertex(glm::vec3, glm::vec2);

    public:

        glm::vec3 Position;
        glm::vec2 TexCoord;
    };

private:

           const float TERRAIN_WIDTH        = 64.0;
           const float TERRAIN_AMPLITUDE    = 30.0f;

           const float DISTANCE_FOR_DETAILS = 128.0f;
           const float MAX_TESSELATION      = 40.0f;

           const float SPECULAR_STRENGTH    = 10.5f;

    static const int   COLORS_COUNT         = 5;

           const glm::vec4 TERRAIN_COLORS[COLORS_COUNT] =
           { 
               glm::vec4(0.003f, 0.047f, 0.188f, 1.0f),
               glm::vec4(0.729f, 0.764f, 0.878f, 1.0f),
               glm::vec4(0.819f, 0.882f, 1.0f, 1.0f),
               glm::vec4(0.596f, 0.831f, 1.0f, 1.0f),
               glm::vec4(0.152f, 0.658f, 0.968f, 1.0f)
           };

    static const int   TERRAIN_GRID_WIDTH   = 64;
    static const int   TERRAIN_GRID_HEIGHT  = 64;

    static const int   INDICES_COUNT        = TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT * 6;

public:

    Terrain(PerlinNoise*);
    ~Terrain();

    void Draw(Light*, Camera*);

private:

    void CreateTerrainBuffers();
    void FreeTerrainBuffers();

    void CreateColorsBuffer();
    void FreeColorsBuffer();

private:

    unsigned int   m_vbo;
    unsigned int   m_ebo;
    unsigned int   m_vao;

    unsigned int   m_colorsBuffer;

    Shader*        m_shader;
    PerlinNoise*   m_perlinNoise;
    RenderTexture* m_renderTexture;

    Texture*       m_texture;
};