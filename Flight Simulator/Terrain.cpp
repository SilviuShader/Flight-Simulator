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
    m_perlinNoise(perlinNoise),
    m_materials(vector<Material*>())
{
    CreateTerrainBuffers();

    m_shader = new Shader("Shaders/Terrain.vert", "Shaders/Terrain.frag", 
        "Shaders/Terrain.tesc", "Shaders/Terrain.tese");

    m_materials.push_back(new Material("Assets/snow_02_diff_1k.png", "Assets/snow_02_nor_gl_1k.png", "Assets/snow_02_spec_1k.png"));
    m_materials.push_back(new Material("Assets/medieval_blocks_02_diff_1k.png", "Assets/medieval_blocks_02_nor_gl_1k.png", "Assets/medieval_blocks_02_spec_1k.png"));

    m_renderTexture = m_perlinNoise->RenderNoise(vec2(0.0f, 0.0f), vec2(TERRAIN_WIDTH, TERRAIN_WIDTH));
}

Terrain::~Terrain()
{
    if (m_renderTexture)
    {
        delete m_renderTexture;
        m_renderTexture = nullptr;
    }

    for (auto& material : m_materials)
    {
        if (material)
        {
            delete material;
            material = nullptr;
        }
    }
    
    m_materials.clear();
    
    if (m_shader)
    {
        delete m_shader;
        m_shader = nullptr;
    }

    FreeTerrainBuffers();
}

void Terrain::Draw(Light* light, Camera* camera)
{
    vec3 cameraPosition = camera->GetPosition();

    mat4 model      = mat4(1.0f);
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = camera->GetProjectionMatrix();
    
    m_shader->Use();

    m_shader->SetMatrix4("Model", model);

    m_shader->SetVec3("CameraPosition", cameraPosition);
    m_shader->SetFloat("DistanceForDetails", DISTANCE_FOR_DETAILS);
    m_shader->SetFloat("TessellationLevel", MAX_TESSELATION);

    m_shader->SetTexture("NoiseTexture", m_renderTexture->GetTexture(), 0);
    m_shader->SetMatrix4("View", view);
    m_shader->SetMatrix4("Projection", projection);

    m_shader->SetFloat("TerrainWidth", TERRAIN_WIDTH);
    m_shader->SetFloat("GridWidth", TERRAIN_GRID_WIDTH);
    m_shader->SetFloat("GridHeight", TERRAIN_GRID_HEIGHT);
    m_shader->SetFloat("TerrainAmplitude", TERRAIN_AMPLITUDE);

    m_shader->SetFloat("Gamma", GAMMA);

    m_shader->SetVec4("AmbientColor", light->GetAmbientColor());
    m_shader->SetVec4("DiffuseColor", light->GetDiffuseColor());
    m_shader->SetVec3("LightDirection", light->GetLightDirection());
    m_shader->SetFloat("SpecularPower", light->GetSpecularPower());

    m_shader->SetVec3("CameraPosition", camera->GetPosition());

    m_shader->SetMaterials("TerrainTextures", "TerrainNormalTextures", "TerrainSpecularTextures", m_materials, 1);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0);
}

void Terrain::CreateTerrainBuffers()
{
    constexpr int verticesWidth = TERRAIN_GRID_WIDTH + 1;
    constexpr int verticesHeight = TERRAIN_GRID_HEIGHT + 1;

    constexpr int verticesCount = verticesWidth * verticesHeight;

    Vertex* vertices = new Vertex[verticesCount];

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (i - ((float)(verticesWidth - 1) / 2.0f)) / (float)(verticesWidth - 1);
            float adjustedJ = (j - ((float)(verticesHeight - 1) / 2.0f)) / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedJ * TERRAIN_WIDTH, adjustedI * TERRAIN_WIDTH);

            vertices[i * verticesWidth + j].Position = vec3(planePosition.x, 0.0f, planePosition.y);
            vertices[i * verticesWidth + j].TexCoord = vec2(j * TEX_COORDS_MULTIPLIER, verticesHeight - i * TEX_COORDS_MULTIPLIER - 1);
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

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

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
}

void Terrain::FreeTerrainBuffers()
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
