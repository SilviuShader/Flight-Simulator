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

    struct MeshData
    {
    public:

        glm::vec3 Position;
    };

private:

           const float TERRAIN_WIDTH        = 64.0;
           const float TERRAIN_AMPLITUDE    = 30.0f;

           const float DISTANCE_FOR_DETAILS = 128.0f;
           const float MAX_TESSELATION      = 40.0f;

    static const int   TERRAIN_GRID_WIDTH   = 64;
    static const int   TERRAIN_GRID_HEIGHT  = 64;

    static const int   INDICES_COUNT        = TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT * 6;

public:

    Terrain(PerlinNoise*);
    ~Terrain();

    void Draw(Light*, Camera*);

private:

    void CreateBuffers();
    void FreeBuffers();

    void FillMeshPositions(MeshData*);

private:

    unsigned int   m_vbo;
    unsigned int   m_ebo;
    unsigned int   m_vao;

    Shader*        m_shader;
    PerlinNoise*   m_perlinNoise;
    RenderTexture* m_renderTexture;

    Texture*     m_texture;
};