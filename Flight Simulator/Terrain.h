#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Terrain
{
private:

    struct Vertex 
    {
    public:

        Vertex(glm::vec3, glm::vec3);

    public:

        glm::vec3 Position;
        glm::vec3 Color;
    };

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

    int m_indicesCount;

    Shader* m_shader;
};