#include "glad/glad.h"

#include "Terrain.h"

using namespace glm;

Terrain::Vertex::Vertex(vec3 position, vec3 color) : 
    Position(position),
    Color(color)
{
}

Terrain::Terrain() :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_indicesCount(0)
{
    CreateBuffers();

    m_shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
}

Terrain::~Terrain()
{
    FreeBuffers();

    if (m_shader)
    {
        delete m_shader;
        m_shader = nullptr;
    }
}

void Terrain::Draw(mat4& viewMatrix, mat4& projectionMatrix)
{
    mat4 model = mat4(1.0f);
    
    m_shader->Use();
    m_shader->SetMatrix4("Model", model);
    m_shader->SetMatrix4("View", viewMatrix);
    m_shader->SetMatrix4("Projection", projectionMatrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, 0);
}

void Terrain::CreateBuffers()
{
    int verticesCount = 36;

    Vertex vertices[] = 
    {
        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f, -0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 0.0f, 1.0f)),

        Vertex(vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f,  0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),

        Vertex(vec3(-0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),

        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),

        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f, -0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f, -0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),

        Vertex(vec3(-0.5f,  0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f, -0.5f),  vec3(1.0f, 1.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(0.5f,  0.5f,  0.5f),  vec3(1.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f,  0.5f),  vec3(0.0f, 0.0f, 1.0f)),
        Vertex(vec3(-0.5f,  0.5f, -0.5f),  vec3(0.0f, 1.0f, 1.0f))
    };

    m_indicesCount = verticesCount;

    unsigned int* indices = new unsigned int[m_indicesCount];

    for (int i = 0; i < m_indicesCount; i++)
        indices[i] = i;

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indicesCount, indices, GL_STATIC_DRAW);;

    delete[] indices;
    indices = nullptr;
}

void Terrain::FreeBuffers()
{
    glBindVertexArray(m_vao);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_vao);
}