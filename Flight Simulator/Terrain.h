#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Terrain
{
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

    Shader* m_shader;
};