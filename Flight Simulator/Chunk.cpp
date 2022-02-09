#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include "glad/glad.h"

#include "Chunk.h"
#include "Shapes.h"
#include "VertexTypes.h"

using namespace std;
using namespace glm;

const float Chunk::CHUNK_WIDTH      = 64.0f;
const float Chunk::CHUNK_CLOSE_BIAS = 1.0f;

Chunk::Node::Node()
{
    memset(Children, 0, sizeof(Node*) * CHILDREN_COUNT);

    IsLeaf      = false;
    ZoneRange   = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    PositionId  = make_pair(0, 0);
    BoundingBox = MathHelper::AABB();
}

Chunk::Node::~Node()
{
    if (IsLeaf)
        return;

    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        if (Children[i])
        {
            delete Children[i];
            Children[i] = nullptr;
        }
    }
}

Chunk::Chunk(PerlinNoise* perlinNoise, Shader* terrainShader, pair<int, int> chunkID) :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_terrainShader(terrainShader),
    m_perlinNoise(perlinNoise),
    m_chunkID(chunkID),
    m_quadTree(nullptr),
    m_camera(nullptr),
    m_renderDebug(false)
{
    CreateTerrainBuffers();

    vec3 translation = GetTranslation();

    auto noiseData = m_perlinNoise->RenderNoise(vec2(translation.x - CHUNK_WIDTH / 2.0f, translation.z - CHUNK_WIDTH / 2.0f),
                                                vec2(translation.x + CHUNK_WIDTH / 2.0f, translation.z + CHUNK_WIDTH / 2.0f),
                                                QUAD_TREE_DEPTH);

    m_noiseTexture = noiseData.first;

    BuildQuadTree(noiseData.second);

    int divisionsCount = 1 << (QUAD_TREE_DEPTH - 1);

    m_drawZonesRanges = new vec4[divisionsCount * divisionsCount];

    for (int i = 0; i < divisionsCount; i++)
    {
        if (noiseData.second[i])
        {
            delete[] noiseData.second[i];
            noiseData.second[i] = nullptr;
        }
    }

    if (noiseData.second)
    {
        delete[] noiseData.second;
        noiseData.second = nullptr;
    }
}

Chunk::~Chunk()
{
    if (m_drawZonesRanges)
    {
        delete[] m_drawZonesRanges;
        m_drawZonesRanges = nullptr;
    }

    if (m_quadTree)
    {
        delete m_quadTree;
        m_quadTree = nullptr;
    }

    if (m_noiseTexture)
    {
        delete m_noiseTexture;
        m_noiseTexture = nullptr;
    }

    FreeTerrainBuffers();
}

void Chunk::Update(Camera* camera, float deltaTime, bool renderDebug)
{
    m_camera      = camera;
    m_renderDebug = renderDebug;

    m_zoneRangesIndex = 0;

    FillZoneRanges(MathHelper::GetCameraFrustum(camera), m_quadTree);
    UpdateZoneRangesBuffer();
}

void Chunk::Draw(Light* light, const vector<Material*>& terrainMaterials, Texture* terrainBiomesData)
{
    vec3 cameraPosition = m_camera->GetPosition();

    mat4 model          = translate(mat4(1.0f), GetTranslation());
    mat4 view           = m_camera->GetViewMatrix();
    mat4 projection     = m_camera->GetProjectionMatrix();

    m_terrainShader->Use();

    m_terrainShader->SetMatrix4("Model",                     model);

    m_terrainShader->SetVec3("CameraPosition",               cameraPosition);
    m_terrainShader->SetFloat("DistanceForDetails",          DISTANCE_FOR_DETAILS);
    m_terrainShader->SetFloat("TessellationLevel",           MAX_TESSELATION);

    m_terrainShader->SetTexture("NoiseTexture",              m_noiseTexture, 0);
    m_terrainShader->SetMatrix4("View",                      view);
    m_terrainShader->SetMatrix4("Projection",                projection);

    m_terrainShader->SetFloat("TerrainWidth",                CHUNK_WIDTH);
    m_terrainShader->SetFloat("GridWidth",                   CHUNK_GRID_WIDTH);
    m_terrainShader->SetFloat("GridHeight",                  CHUNK_GRID_HEIGHT);
    m_terrainShader->SetFloat("TerrainAmplitude",            TERRAIN_AMPLITUDE);

    m_terrainShader->SetFloat("Gamma",                       GAMMA);

    m_terrainShader->SetVec4("AmbientColor",                 light->GetAmbientColor());
    m_terrainShader->SetVec4("DiffuseColor",                 light->GetDiffuseColor());
    m_terrainShader->SetVec3("LightDirection",               light->GetLightDirection());
    m_terrainShader->SetFloat("SpecularPower",               light->GetSpecularPower());

    m_terrainShader->SetVec3("CameraPosition",               m_camera->GetPosition());

    m_terrainShader->SetInt("BiomesCount",                   terrainBiomesData->GetWidth());
    m_terrainShader->SetInt("MaterialsPerBiome",             terrainBiomesData->GetHeight());

    m_terrainShader->SetTexture("BiomeMaterialsTexture",     terrainBiomesData, 1);

    m_terrainShader->SetMaterials("TerrainTextures", 
                                  "TerrainNormalTextures", 
                                  "TerrainSpecularTextures", terrainMaterials, 2);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElementsInstanced(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0, m_zoneRangesIndex);
}

void Chunk::CreateTerrainBuffers()
{
    constexpr int verticesWidth     = CHUNK_GRID_WIDTH + 1;
    constexpr int verticesHeight    = CHUNK_GRID_HEIGHT + 1;

    constexpr int verticesCount     = verticesWidth * verticesHeight;

    VertexPositionTexture* vertices = new VertexPositionTexture[verticesCount];

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (float)i / (float)(verticesWidth  - 1);
            float adjustedJ = (float)j / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedJ, adjustedI);

            vertices[i * verticesWidth + j].Position = vec3(planePosition.x, 0.0f, planePosition.y);
            vertices[i * verticesWidth + j].TexCoord = vec2(j * TEX_COORDS_MULTIPLIER, verticesHeight - i * TEX_COORDS_MULTIPLIER - 1);
        }
    }

    unsigned int* indices = new unsigned int[INDICES_COUNT];

    int indicesIndex      = 0;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPositionTexture) * verticesCount, vertices, GL_STATIC_DRAW);

    VertexPositionTexture::SetLayout();

    glGenBuffers(1, &m_instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

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

    VertexPositionTexture::ResetLayout();
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_instanceVbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_vao);
}

void Chunk::BuildQuadTree(PerlinNoise::MinMax** minMax)
{
    m_quadTree = CreateNode(0, 
                            vec2(-CHUNK_WIDTH / 2.0f, -CHUNK_WIDTH / 2.0f), 
                            vec2(CHUNK_WIDTH / 2.0f, CHUNK_WIDTH / 2.0f), 
                            make_pair(0, 0),
                            minMax);
}

Chunk::Node* Chunk::CreateNode(int depth, const vec2& bottomLeft, const vec2& topRight, pair<int, int> positionId, PerlinNoise::MinMax** minMax)
{
    if (depth >= QUAD_TREE_DEPTH)
        return nullptr;

    Node* result            = new Node();

    result->ZoneRange       = vec4(bottomLeft.x, bottomLeft.y, topRight.x, topRight.y);
    result->PositionId      = positionId;

    vec3 boundingBoxCenter  = vec3((bottomLeft.x + topRight.x) * 0.5f, 0.0f,
                                   (bottomLeft.y + topRight.y) * 0.5f) + GetTranslation();

    vec3 boundingBoxExtents = vec3((topRight.x - bottomLeft.x) * 0.5f, TERRAIN_AMPLITUDE, 
                                   (topRight.y - bottomLeft.y) * 0.5f);

    if (depth == QUAD_TREE_DEPTH - 1)
    {
        result->IsLeaf = true;

        float minAmplitude = minMax[positionId.first][positionId.second].first  * TERRAIN_AMPLITUDE;
        float maxAmplitude = minMax[positionId.first][positionId.second].second * TERRAIN_AMPLITUDE;

        float center       = (maxAmplitude + minAmplitude) / 2.0f;
        float extents      = (maxAmplitude - minAmplitude) / 2.0f;

        boundingBoxCenter  = vec3(boundingBoxCenter.x,  center,  boundingBoxCenter.z);
        boundingBoxExtents = vec3(boundingBoxExtents.x, extents, boundingBoxExtents.z);
    }
    else
    {
        result->IsLeaf      = false;
        result->Children[0] = CreateNode(depth + 1,
                                         bottomLeft,
                                         (topRight + bottomLeft) * 0.5f,
                                         make_pair(positionId.first * 2, positionId.second * 2),
                                         minMax);

        result->Children[1] = CreateNode(depth + 1,
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, bottomLeft.y),
                                         vec2(topRight.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         make_pair(1 + positionId.first * 2, positionId.second * 2),
                                         minMax);

        result->Children[2] = CreateNode(depth + 1,
                                         vec2(bottomLeft.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, topRight.y),
                                         make_pair(positionId.first * 2, positionId.second * 2 + 1),
                                         minMax);

        result->Children[3] = CreateNode(depth + 1,
                                         (topRight + bottomLeft) * 0.5f,
                                         topRight,
                                         make_pair(positionId.first * 2 + 1, positionId.second * 2 + 1),
                                         minMax);

        float maxAmplitude  = -TERRAIN_AMPLITUDE;
        float minAmplitude  = TERRAIN_AMPLITUDE;

        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
        {
            maxAmplitude = std::max(maxAmplitude, 
                                    result->Children[i]->BoundingBox.Center.y + result->Children[i]->BoundingBox.Extents.y);

            minAmplitude = std::min(maxAmplitude, 
                                    result->Children[i]->BoundingBox.Center.y - result->Children[i]->BoundingBox.Extents.y);
        }

        float center       = (maxAmplitude + minAmplitude) / 2.0f;
        float extents      = (maxAmplitude - minAmplitude) / 2.0f;

        boundingBoxCenter  = vec3(boundingBoxCenter.x,  center,  boundingBoxCenter.z);
        boundingBoxExtents = vec3(boundingBoxExtents.x, extents, boundingBoxExtents.z);
    }

    result->BoundingBox = MathHelper::AABB(boundingBoxCenter, 
                                           boundingBoxExtents.x, boundingBoxExtents.y, boundingBoxExtents.z);

    return result;
}

void Chunk::FillZoneRanges(const MathHelper::Frustum& frustum, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;
    
    if (node->IsLeaf)
    {
        m_drawZonesRanges[m_zoneRangesIndex++] = node->ZoneRange;

        if (m_renderDebug)
            Shapes::GetInstance()->AddInstance(node->BoundingBox.Center, node->BoundingBox.Extents);
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            FillZoneRanges(frustum, node->Children[i]);
    }
}

void Chunk::UpdateZoneRangesBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * m_zoneRangesIndex, m_drawZonesRanges, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

vec3 Chunk::GetTranslation() const
{
    return vec3(m_chunkID.first  * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS), 
                0.0f, 
                m_chunkID.second * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS));
}