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
        glm::vec3 Normal;
    };

    struct MeshData
    {
    public:

        glm::vec3 Position;
        glm::vec3 Normal;
    };

private:

           const float TERRAIN_WIDTH        = 128.0f;
           const float TERRAIN_AMPLITUDE    = 60.0f;

           const float DISTANCE_FOR_DETAILS = 50.0f;
           const float MAX_TESSELATION      = 10.0f;

    static const int   TERRAIN_GRID_WIDTH   = 128;
    static const int   TERRAIN_GRID_HEIGHT  = 128;

    static const int   INDICES_COUNT        = TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT * 6;

public:

    Terrain(PerlinNoise*);
    ~Terrain();

    void Draw(Light*, Camera*);

private:

    void CreateBuffers();
    void FreeBuffers();

    void FillMeshPositions(MeshData*);
    void FillMeshNormals(MeshData*);

private:

    unsigned int m_vbo;
    unsigned int m_ebo;
    unsigned int m_vao;

    Shader*      m_shader;
    PerlinNoise* m_perlinNoise;

    Texture*     m_texture;
};