#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

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

           const float TERRAIN_WIDTH         = 64.0;
           const float TERRAIN_AMPLITUDE     = 75.0f;

           const float DISTANCE_FOR_DETAILS  = 128.0f;
           const float MAX_TESSELATION       = 10.0f;
           const float TEX_COORDS_MULTIPLIER = 0.1f;

           const float GAMMA                 = 1.0f;

    static const int   COLORS_COUNT          = 5;

    static const int   TERRAIN_GRID_WIDTH   = 256;
    static const int   TERRAIN_GRID_HEIGHT  = 256;

    static const int   INDICES_COUNT        = TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT * 6;

public:

    Terrain(PerlinNoise*);
    ~Terrain();

    void Draw(Light*, Camera*);

private:

    void CreateTerrainBuffers();
    void FreeTerrainBuffers();

private:

    unsigned int           m_vbo;
    unsigned int           m_ebo;
    unsigned int           m_vao;
                           
    Shader*                m_shader;
    PerlinNoise*           m_perlinNoise;
    RenderTexture*         m_renderTexture;

    std::vector<Material*> m_materials;
};