#include <memory>
#include "glad/glad.h"

#include "Terrain.h"

using namespace std;
using namespace glm;

Terrain::Vertex::Vertex() :
    Position(vec3(0.0f, 0.0f, 0.0f)),
    TexCoord(vec2(0.0f, 0.0f))
{
}

Terrain::Vertex::Vertex(vec3 position, vec2 texCoord) :
    Position(position),
    TexCoord(texCoord)
{
}

Terrain::Terrain(PerlinNoise* perlinNoise) :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_shader(nullptr),
    m_perlinNoise(perlinNoise)
{
    CreateBuffers();

    m_shader = new Shader("Shaders/Terrain.vert", "Shaders/Terrain.frag", 
        "Shaders/Terrain.tesc", "Shaders/Terrain.tese");

    m_texture = new Texture("Assets/dirt01d.tga");
}

Terrain::~Terrain()
{
    if (m_texture)
    {
        delete m_texture;
        m_texture = nullptr;
    }

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

    m_shader->SetVec4("DiffuseColor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_shader->SetVec3("LightDirection", vec3(1.0f, -1.0f, 0.0f));
    m_shader->SetTexture("TerrainTexture", m_texture, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0);
}

void Terrain::CreateBuffers()
{
    constexpr int verticesWidth = TERRAIN_GRID_WIDTH + 1;
    constexpr int verticesHeight = TERRAIN_GRID_HEIGHT + 1;

    constexpr int verticesCount = verticesWidth * verticesHeight;

    MeshData* meshData = new MeshData[verticesCount];

    FillMeshPositions(meshData);
    FillMeshNormals(meshData);

    Vertex* vertices = new Vertex[verticesCount];

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            vertices[i * verticesWidth + j].Position = meshData[i * verticesWidth + j].Position;
            vertices[i * verticesWidth + j].TexCoord = vec2(j % 2 == 0 ? 0.0f : 1.0f, i % 2 == 0 ? 0.0f : 1.0f);
            vertices[i * verticesWidth + j].Normal   = meshData[i * verticesWidth + j].Normal;
        }
    }

    unsigned int* indices = new unsigned int[INDICES_COUNT];

    int indicesIndex = 0;

    for (int i = 0; i < TERRAIN_GRID_HEIGHT; i++)
    {
        for (int j = 0; j < TERRAIN_GRID_WIDTH; j++)
        {
            int pivot = i * verticesWidth + j;
            int right = i * verticesHeight + j + 1;
            int bottom = (i + 1) * verticesWidth + j;
            int bottomRight = (i + 1) * verticesHeight + j + 1;

            indices[indicesIndex++] = pivot;
            indices[indicesIndex++] = right;
            indices[indicesIndex++] = bottom;

            indices[indicesIndex++] = right;
            indices[indicesIndex++] = bottomRight;
            indices[indicesIndex++] = bottom;
        }
    }

    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3) + sizeof(vec2)));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &m_ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * INDICES_COUNT, indices, GL_STATIC_DRAW);

    if (indices)
    {
        delete[] indices;
        indices = nullptr;
    }

    if (vertices)
    {
        delete[] vertices;
        vertices = nullptr;
    }

    if (meshData)
    {
        delete[] meshData;
        meshData = nullptr;
    }
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

void Terrain::FillMeshPositions(MeshData* meshData)
{
    int verticesWidth = TERRAIN_GRID_WIDTH + 1;
    int verticesHeight = TERRAIN_GRID_HEIGHT + 1;

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (i - ((float)(verticesWidth - 1) / 2.0f)) / (float)(verticesWidth - 1);
            float adjustedJ = (j - ((float)(verticesHeight - 1) / 2.0f)) / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedI * TERRAIN_WIDTH, adjustedJ * TERRAIN_WIDTH);
            float height = m_perlinNoise->GetCombinedValue(planePosition) * TERRAIN_AMPLITUDE;

            meshData[i * verticesWidth + j].Position = vec3(planePosition.x, height, planePosition.y);
        }
    }
}

void Terrain::FillMeshNormals(MeshData* meshData)
{
    int verticesWidth = TERRAIN_GRID_WIDTH + 1;
    int verticesHeight = TERRAIN_GRID_HEIGHT + 1;

    vec3 faceNormals[TERRAIN_GRID_WIDTH * TERRAIN_GRID_HEIGHT];

    for (int i = 0; i < TERRAIN_GRID_HEIGHT; i++)
    {
        for (int j = 0; j < TERRAIN_GRID_WIDTH; j++)
        {
            vec3 bottomLeftPos = meshData[i * verticesWidth + j].Position;
            vec3 topLeftPos = meshData[(i + 1) * verticesWidth + j].Position;
            vec3 bottomRightPos = meshData[i * verticesWidth + j + 1].Position;

            vec3 toTopLeft     = topLeftPos - bottomLeftPos;
            vec3 toBottomRight = bottomRightPos - bottomLeftPos;

            vec3 crossProduct = cross(toBottomRight, toTopLeft);

            vec3 normal = normalize(crossProduct);

            faceNormals[i * TERRAIN_GRID_WIDTH + j] = normal;
        }
    }

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            vec3 normalSum = vec3(0.0f, 0.0f, 0.0f);

            // top-right face
            if (i < TERRAIN_GRID_HEIGHT && j < TERRAIN_GRID_WIDTH)
                normalSum = normalSum + faceNormals[i * TERRAIN_GRID_WIDTH + j];

            // top-left face
            if (i < TERRAIN_GRID_HEIGHT && j >= 1)
                normalSum = normalSum + faceNormals[i * TERRAIN_GRID_WIDTH + j - 1];

            // bottom-left face
            if (i >= 1 && j >= 1)
                normalSum = normalSum + faceNormals[(i - 1) * TERRAIN_GRID_WIDTH + j - 1];

            // bottom-right face
            if (i >= 1 && j < TERRAIN_GRID_WIDTH)
                normalSum = normalSum + faceNormals[(i - 1) * TERRAIN_GRID_WIDTH + j];

            vec3 normal = normalize(normalSum);
            meshData[i * verticesWidth + j].Normal = normal;
        }
    }
}
