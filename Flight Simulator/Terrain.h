#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"

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

    const int   TERRAIN_GRID_WIDTH   = 32;
    const int   TERRAIN_GRID_HEIGHT  = 32;

    const float TERRAIN_WIDTH        = 64.0f;
    const float TERRAIN_AMPLITUDE    = 20.0f;

    const float DISTANCE_FOR_DETAILS = 50.0f;
    const float MAX_TESSELATION      = 20.0f;

public:

    Terrain(PerlinNoise*);
    ~Terrain();

    void Draw(Camera*);

private:

    void CreateBuffers();
    void FreeBuffers();

private:

    unsigned int m_vbo;
    unsigned int m_ebo;
    unsigned int m_vao;

    int          m_indicesCount;

    Shader*      m_shader;
    PerlinNoise* m_perlinNoise;

    Texture*     m_texture;
};