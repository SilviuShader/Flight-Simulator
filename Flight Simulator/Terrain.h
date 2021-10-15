#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"

class Terrain
{
private:

    struct Vertex 
    {
    public:

        Vertex();
        Vertex(glm::vec3, glm::vec3);

    public:

        glm::vec3 Position;
        glm::vec3 Color;
    };

private:

    const int   TERRAIN_GRID_WIDTH  = 256;
    const int   TERRAIN_GRID_HEIGHT = 256;

    const float TERRAIN_WIDTH       = 64.0f;
    const float TERRAIN_AMPLITUDE   = 10.0f;

public:

    Terrain();
    ~Terrain();

    void Draw(glm::mat4&, glm::mat4&);

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
};