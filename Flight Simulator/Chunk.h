#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

class Chunk
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

           const float CHUNK_WIDTH           = 64.0;
           const float TERRAIN_AMPLITUDE     = 75.0f;

           const float DISTANCE_FOR_DETAILS  = 64.0f;
           const float MAX_TESSELATION       = 5.0f;
           const float TEX_COORDS_MULTIPLIER = 0.2f;

           const float GAMMA                 = 1.0f;

           const float BIOMES_COUNT          = 2;
           const float MATERIALS_PER_BIOME   = 4;

    static const int   COLORS_COUNT          = 5;

    static const int   CHUNK_GRID_WIDTH      = 128;
    static const int   CHUNK_GRID_HEIGHT     = 128;

    static const int   INDICES_COUNT        = CHUNK_GRID_WIDTH * CHUNK_GRID_HEIGHT * 6;
public:

    Chunk(PerlinNoise*);
    ~Chunk();

    void Draw(Light*, Camera*);

private:

    void CreateTerrainBuffers();
    void FreeTerrainBuffers();

private:

    unsigned int           m_vbo;
    unsigned int           m_ebo;
    unsigned int           m_vao;
                           
    Shader*                m_terrainShader;
    PerlinNoise*           m_perlinNoise;
    RenderTexture*         m_renderTexture;

    std::vector<Material*> m_materials;
    Texture*               m_biomeMaterialsTexture;
};