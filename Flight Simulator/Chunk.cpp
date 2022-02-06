#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include "glad/glad.h"

#include "Chunk.h"
#include "Shapes.h"

using namespace std;
using namespace glm;

const float Chunk::CHUNK_WIDTH      = 64.0f;
const float Chunk::CHUNK_CLOSE_BIAS = 1.0f;

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
    m_quadTree(nullptr)
{
    CreateTerrainBuffers();

    vec3 translation = GetTranslation();

    auto noiseData = m_perlinNoise->RenderNoise(vec2(translation.x - CHUNK_WIDTH / 2.0f, translation.z - CHUNK_WIDTH / 2.0f),
                                                vec2(translation.x + CHUNK_WIDTH / 2.0f, translation.z + CHUNK_WIDTH / 2.0f),
                                                QUAD_TREE_DEPTH);

    m_renderTexture = noiseData.first;

    BuildQuadTree(noiseData.second);
}

Chunk::~Chunk()
{
    if (m_quadTree)
    {
        delete m_quadTree;
        m_quadTree = nullptr;
    }

    if (m_renderTexture)
    {
        delete m_renderTexture;
        m_renderTexture = nullptr;
    }

    FreeTerrainBuffers();
}

void Chunk::Draw(Light* light, Camera* camera, const vector<Material*>& terrainMaterials, Texture* terrainBiomesData)
{
    vec3 cameraPosition = camera->GetPosition();

    mat4 model      = translate(mat4(1.0f), GetTranslation());
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

    m_terrainShader->SetInt("BiomesCount", terrainBiomesData->GetWidth());
    m_terrainShader->SetInt("MaterialsPerBiome", terrainBiomesData->GetHeight());

    m_terrainShader->SetTexture("BiomeMaterialsTexture", terrainBiomesData, 1);

    m_terrainShader->SetMaterials("TerrainTextures", "TerrainNormalTextures", "TerrainSpecularTextures", terrainMaterials, 2);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    DrawNode(MathHelper::GetCameraFrustum(camera), m_quadTree);
    //DrawQuadTrees(MathHelper::GetCameraFrustum(camera), camera, m_quadTree);
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
            float adjustedI = (float)i / (float)(verticesWidth - 1);
            float adjustedJ = (float)j / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedJ, adjustedI);

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

void Chunk::BuildQuadTree(map<pair<int, int>, pair<float, float>>& minMax)
{
    m_quadTree = CreateNode(0, 
                            vec2(-CHUNK_WIDTH / 2.0f, -CHUNK_WIDTH / 2.0f), 
                            vec2(CHUNK_WIDTH / 2.0f, CHUNK_WIDTH / 2.0f), 
                            make_pair(0, 0),
                            minMax);
}

void Chunk::DrawNode(const MathHelper::Frustum& frustum, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;
    
    if (node->IsLeaf)
    {
        m_terrainShader->SetVec2("BottomLeft", node->BottomLeft);
        m_terrainShader->SetVec2("TopRight", node->TopRight);
        glDrawElements(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0);
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            DrawNode(frustum, node->Children[i]);
    }
}

void Chunk::DrawQuadTrees(const MathHelper::Frustum& frustum, Camera* camera, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;

    if (node->IsLeaf)
    {
        auto& boundingBox = node->BoundingBox;
        Shapes::GetInstance()->DrawRectangle(boundingBox.Center, boundingBox.Extents, camera);
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            DrawQuadTrees(frustum, camera, node->Children[i]);
    }
}

vec3 Chunk::GetTranslation() const
{
    return vec3(m_chunkID.first * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS), 0.0f, m_chunkID.second * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS));
}

Chunk::Node* Chunk::CreateNode(int depth, const vec2& bottomLeft, const vec2& topRight, pair<int, int> positionId, map<pair<int, int>, pair<float, float>>& minMax)
{
    if (depth >= QUAD_TREE_DEPTH)
        return nullptr;

    Node* result = new Node();

    result->BottomLeft = bottomLeft;
    result->TopRight   = topRight;
    result->PositionId = positionId;

    vec3 boundingBoxCenter = vec3((bottomLeft.x + topRight.x) * 0.5f, 0.0f, (bottomLeft.y + topRight.y) * 0.5f) + GetTranslation();
    vec3 boundingBoxExtents = vec3((topRight.x - bottomLeft.x) * 0.5f, TERRAIN_AMPLITUDE, (topRight.y - bottomLeft.y) * 0.5f);

    if (depth == QUAD_TREE_DEPTH - 1)
    {
        result->IsLeaf = true;

        float minAmplitude = minMax[positionId].first * TERRAIN_AMPLITUDE;
        float maxAmplitude = minMax[positionId].second * TERRAIN_AMPLITUDE;

        float center = (maxAmplitude + minAmplitude) / 2.0f;
        float extents = (maxAmplitude - minAmplitude) / 2.0f;

        boundingBoxCenter = vec3(boundingBoxCenter.x, center, boundingBoxCenter.z);
        boundingBoxExtents = vec3(boundingBoxExtents.x, extents, boundingBoxExtents.z);

        memset(result->Children, 0, sizeof(Node*) * Node::CHILDREN_COUNT);
    }
    else
    {
        result->IsLeaf = false;
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

        float maxAmplitude = -TERRAIN_AMPLITUDE;
        float minAmplitude = TERRAIN_AMPLITUDE;

        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
        {
            maxAmplitude = std::max(maxAmplitude, result->Children[i]->BoundingBox.Center.y + result->Children[i]->BoundingBox.Extents.y);
            minAmplitude = std::min(maxAmplitude, result->Children[i]->BoundingBox.Center.y - result->Children[i]->BoundingBox.Extents.y);
        }

        float center = (maxAmplitude + minAmplitude) / 2.0f;
        float extents = (maxAmplitude - minAmplitude) / 2.0f;

        boundingBoxCenter = vec3(boundingBoxCenter.x, center, boundingBoxCenter.z);
        boundingBoxExtents = vec3(boundingBoxExtents.x, extents, boundingBoxExtents.z);
    }

    result->BoundingBox = MathHelper::AABB(boundingBoxCenter, boundingBoxExtents.x, boundingBoxExtents.y, boundingBoxExtents.z);

    return result;
}
