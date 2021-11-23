#include <memory>
#include "glad/glad.h"

#include "Chunk.h"

using namespace std;
using namespace glm;

Chunk::Vertex::Vertex() :
    Position(vec3(0.0f, 0.0f, 0.0f)),
    TexCoord(vec2(0.0f, 0.0f))
{
}

Chunk::Vertex::Vertex(vec3 position, vec2 texCoord) :
    Position(position),
    TexCoord(texCoord)
{
}

Chunk::Chunk(PerlinNoise* perlinNoise) :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_terrainShader(nullptr),
    m_perlinNoise(perlinNoise),
    m_materials(vector<Material*>()),
    m_biomeMaterialsTexture(nullptr)
{
    CreateTerrainBuffers();

    m_terrainShader = new Shader("Shaders/Terrain.vert", "Shaders/Terrain.frag",
        "Shaders/Terrain.tesc", "Shaders/Terrain.tese");

    m_materials.push_back(new Material("Assets/snow_02_diff_1k.png", "Assets/snow_02_nor_gl_1k.png", "Assets/snow_02_spec_1k.png"));
    m_materials.push_back(new Material("Assets/medieval_blocks_02_diff_1k.png", "Assets/medieval_blocks_02_nor_gl_1k.png", "Assets/medieval_blocks_02_spec_1k.png"));
    m_materials.push_back(new Material("Assets/brown_mud_leaves_01_diff_1k.png", "Assets/brown_mud_leaves_01_nor_gl_1k.png", "Assets/brown_mud_leaves_01_spec_1k.png"));
    m_materials.push_back(new Material("Assets/forest_leaves_03_diff_1k.png", "Assets/forest_leaves_03_nor_gl_1k.png"));
    m_materials.push_back(new Material("Assets/snow_field_aerial_col_1k.png", "Assets/snow_field_aerial_nor_gl_1k.png"));
    m_materials.push_back(new Material("Assets/snow_03_diff_1k.png", "Assets/snow_03_nor_gl_1k.png", "Assets/snow_03_spec_1k.png"));

    float materialsCount = m_materials.size() - 1;
    float* biomesData = new float[BIOMES_COUNT * MATERIALS_PER_BIOME];
    biomesData[0] = 2.0f / materialsCount;
    biomesData[1] = 3.0f / materialsCount;

    biomesData[2] = 1.0f / materialsCount;
    biomesData[3] = 5.0f / materialsCount;
    
    biomesData[4] = 5.0f / materialsCount;
    biomesData[5] = 1.0f / materialsCount;
    
    biomesData[6] = 0.0f / materialsCount;
    biomesData[7] = 4.0f / materialsCount;

    m_biomeMaterialsTexture = new Texture(biomesData, BIOMES_COUNT, MATERIALS_PER_BIOME);

    if (biomesData)
    {
        delete[] biomesData;
        biomesData = nullptr;
    }

    m_renderTexture = m_perlinNoise->RenderNoise(vec2(0.0f, 0.0f), vec2(CHUNK_WIDTH, CHUNK_WIDTH));
}

Chunk::~Chunk()
{
    if (m_renderTexture)
    {
        delete m_renderTexture;
        m_renderTexture = nullptr;
    }

    if (m_biomeMaterialsTexture)
    {
        delete m_biomeMaterialsTexture;
        m_biomeMaterialsTexture = nullptr;
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
    
    if (m_terrainShader)
    {
        delete m_terrainShader;
        m_terrainShader = nullptr;
    }

    FreeTerrainBuffers();
}

void Chunk::Draw(Light* light, Camera* camera)
{
    vec3 cameraPosition = camera->GetPosition();

    mat4 model      = mat4(1.0f);
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = camera->GetProjectionMatrix();
    
    m_terrainShader->Use();

    m_terrainShader->SetMatrix4("Model", model);

    m_terrainShader->SetVec3("CameraPosition", cameraPosition);
    m_terrainShader->SetFloat("DistanceForDetails", DISTANCE_FOR_DETAILS);
    m_terrainShader->SetFloat("TessellationLevel", MAX_TESSELATION);

    m_terrainShader->SetTexture("NoiseTexture", m_renderTexture->GetTexture(), 0);
    m_terrainShader->SetMatrix4("View", view);
    m_terrainShader->SetMatrix4("Projection", projection);

    m_terrainShader->SetFloat("TerrainWidth", CHUNK_WIDTH);
    m_terrainShader->SetFloat("GridWidth", CHUNK_GRID_WIDTH);
    m_terrainShader->SetFloat("GridHeight", CHUNK_GRID_HEIGHT);
    m_terrainShader->SetFloat("TerrainAmplitude", TERRAIN_AMPLITUDE);

    m_terrainShader->SetFloat("Gamma", GAMMA);

    m_terrainShader->SetVec4("AmbientColor", light->GetAmbientColor());
    m_terrainShader->SetVec4("DiffuseColor", light->GetDiffuseColor());
    m_terrainShader->SetVec3("LightDirection", light->GetLightDirection());
    m_terrainShader->SetFloat("SpecularPower", light->GetSpecularPower());

    m_terrainShader->SetVec3("CameraPosition", camera->GetPosition());

    m_terrainShader->SetInt("BiomesCount", BIOMES_COUNT);
    m_terrainShader->SetInt("MaterialsPerBiome", MATERIALS_PER_BIOME);

    m_terrainShader->SetTexture("BiomeMaterialsTexture", m_biomeMaterialsTexture, 1);

    m_terrainShader->SetMaterials("TerrainTextures", "TerrainNormalTextures", "TerrainSpecularTextures", m_materials, 2);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0);
}

void Chunk::CreateTerrainBuffers()
{
    constexpr int verticesWidth = CHUNK_GRID_WIDTH + 1;
    constexpr int verticesHeight = CHUNK_GRID_HEIGHT + 1;

    constexpr int verticesCount = verticesWidth * verticesHeight;

    Vertex* vertices = new Vertex[verticesCount];

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (i - ((float)(verticesWidth - 1) / 2.0f)) / (float)(verticesWidth - 1);
            float adjustedJ = (j - ((float)(verticesHeight - 1) / 2.0f)) / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedJ * CHUNK_WIDTH, adjustedI * CHUNK_WIDTH);

            vertices[i * verticesWidth + j].Position = vec3(planePosition.x, 0.0f, planePosition.y);
            vertices[i * verticesWidth + j].TexCoord = vec2(j * TEX_COORDS_MULTIPLIER, verticesHeight - i * TEX_COORDS_MULTIPLIER - 1);
        }
    }

    unsigned int* indices = new unsigned int[INDICES_COUNT];

    int indicesIndex = 0;

    for (int i = 0; i < CHUNK_GRID_HEIGHT; i++)
    {
        for (int j = 0; j < CHUNK_GRID_WIDTH; j++)
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

void Chunk::FreeTerrainBuffers()
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
