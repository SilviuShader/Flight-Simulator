#include <memory>
#include "glad/glad.h"

#include "Terrain.h"

using namespace std;
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

Terrain::Terrain(PerlinNoise* perlinNoise) :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_indicesCount(0),
    m_shader(nullptr),
    m_perlinNoise(perlinNoise)
{
    CreateBuffers();

    m_shader = new Shader("Shaders/Terrain.vert", "Shaders/Terrain.frag", 
        "Shaders/Terrain.tesc", "Shaders/Terrain.tese");
}

Terrain::~Terrain()
{
    if (m_shader)
    {
        delete m_shader;
        m_shader = nullptr;
    }

    FreeBuffers();
}

void Terrain::Draw(Camera* camera)
{
    vec3 cameraPosition = camera->GetPosition();

    mat4 model      = mat4(1.0f);
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = camera->GetProjectionMatrix();
    
    m_shader->Use();

    m_shader->SetBlockBinding("NoiseValues", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_perlinNoise->GetNoiseValuesBuffer());

    m_shader->SetMatrix4("Model", model);

    m_shader->SetVec3("CameraPosition", cameraPosition);
    m_shader->SetFloat("DistanceForDetails", DISTANCE_FOR_DETAILS);
    m_shader->SetFloat("TessellationLevel", MAX_TESSELATION);

    m_shader->SetFloat("NoiseDefaultFrequency", PerlinNoise::DEFAULT_FREQUENCY);
    m_shader->SetFloat("TerrainAmplitude", TERRAIN_AMPLITUDE);

    m_shader->SetInt("StartOctave", PerlinNoise::OCTAVES_COUNT);
    m_shader->SetInt("OctavesAdd", PerlinNoise::OCTAVES_COUNT);

    m_shader->SetMatrix4("View", view);
    m_shader->SetMatrix4("Projection", projection);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_PATCHES, m_indicesCount, GL_UNSIGNED_INT, 0);
}

void Terrain::CreateBuffers()
{
    int verticesWidth = TERRAIN_GRID_WIDTH + 1;
    int verticesHeight = TERRAIN_GRID_HEIGHT + 1;

    int verticesCount = verticesWidth * verticesHeight;

    unique_ptr<Vertex> vertices(new Vertex[verticesCount]);

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (i - ((float)(verticesWidth - 1) / 2.0f)) / (float)(verticesWidth - 1);
            float adjustedJ = (j - ((float)(verticesHeight - 1) / 2.0f)) / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedI * TERRAIN_WIDTH, adjustedJ * TERRAIN_WIDTH);
            float height = m_perlinNoise->GetCombinedValue(planePosition) * TERRAIN_AMPLITUDE;

            vertices.get()[i * verticesWidth + j].Position = vec3(planePosition.x, height, planePosition.y);
            vertices.get()[i * verticesWidth + j].Color = vec3(1.0f, 1.0f, 1.0f);
        }
    }

    m_indicesCount = TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT * 6;

    unique_ptr<unsigned int> indices(new unsigned int[m_indicesCount]);

    int indicesIndex = 0;

    for (int i = 0; i < TERRAIN_GRID_HEIGHT; i++)
    {
        for (int j = 0; j < TERRAIN_GRID_WIDTH; j++)
        {
            int pivot = i * verticesWidth + j;
            int right = i * verticesHeight + j + 1;
            int bottom = (i + 1) * verticesWidth + j;
            int bottomRight = (i + 1) * verticesHeight + j + 1;

            indices.get()[indicesIndex++] = pivot;
            indices.get()[indicesIndex++] = right;
            indices.get()[indicesIndex++] = bottom;

            indices.get()[indicesIndex++] = right;
            indices.get()[indicesIndex++] = bottomRight;
            indices.get()[indicesIndex++] = bottom;
        }
    }

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, vertices.get(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indicesCount, indices.get(), GL_STATIC_DRAW);

    indices.reset();
    vertices.reset();
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