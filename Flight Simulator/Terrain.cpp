#include "glad/glad.h"

#include "Terrain.h"

using namespace glm;

Terrain::Vertex::Vertex() :
    Position(vec3(0.0f, 0.0f, 0.0f)),
    Color(vec3(0.0f, 0.0f, 0.0f))
{
}

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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
    int gridWidth = 256;
    int gridHeight = 256;
    int verticesCount = gridWidth * gridHeight;

    float terrainSize = 100.0f;

    Vertex* vertices = new Vertex[gridWidth * gridHeight];

    for (int i = 0; i < gridHeight; i++)
    {
        for (int j = 0; j < gridWidth; j++)
        {
            float adjustedI = (i - ((float)(gridWidth - 1) / 2.0f)) / (float)(gridWidth - 1);
            float adjustedJ = (j - ((float)(gridHeight - 1) / 2.0f)) / (float)(gridHeight - 1);

            vertices[i * gridHeight + j] = Vertex(vec3(adjustedJ * terrainSize, 0.0f, adjustedI * terrainSize), vec3(1.0f, 1.0f, 1.0f));
        }
    }

    m_indicesCount = (gridHeight - 1) * (gridWidth - 1) * 6;

    unsigned int* indices = new unsigned int[m_indicesCount];

    int indicesIndex = 0;

    for (int i = 0; i < gridHeight - 1; i++)
    {
        for (int j = 0; j < gridWidth - 1; j++)
        {
            int pivot = i * gridWidth + j;
            int right = i * gridWidth + j + 1;
            int bottom = (i + 1) * gridWidth + j;
            int bottomRight = (i + 1) * gridWidth + j + 1;

            indices[indicesIndex++] = pivot;
            indices[indicesIndex++] = bottom;
            indices[indicesIndex++] = right;

            indices[indicesIndex++] = right;
            indices[indicesIndex++] = bottom;
            indices[indicesIndex++] = bottomRight;
        }
    }

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