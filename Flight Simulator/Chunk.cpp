#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include "glad/glad.h"

#include "Chunk.h"
#include "Shapes.h"
#include "VertexTypes.h"
#include "Biome.h"
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

const float Chunk::CHUNK_WIDTH      = 64.0f;
const float Chunk::CHUNK_CLOSE_BIAS = 1.0f;

Chunk::Node::Node()
{
    memset(Children, 0, sizeof(Node*) * CHILDREN_COUNT);

    IsLeaf           = false;
    ZoneRange        = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    PositionId       = make_pair(0, 0);
    BoundingBox      = MathHelper::AABB();
    DesiredInstances = unordered_map<Model*, vector<mat4>>();
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

Chunk::Chunk(PerlinNoise* perlinNoise, Shader* terrainShader, pair<int, int> chunkID, Shader* folliageShader) :
    m_vbo(0),
    m_ebo(0),
    m_vao(0),
    m_terrainShader(terrainShader),
    m_perlinNoise(perlinNoise),
    m_chunkID(chunkID),
    m_folliageShader(folliageShader),
    m_quadTree(nullptr),
    m_camera(nullptr),
    m_renderDebug(false)
{
    CreateTerrainBuffers();

    vec3 translation = GetTranslation();

    auto noiseData = m_perlinNoise->RenderNoise(vec2(translation.x - CHUNK_WIDTH / 2.0f, translation.z - CHUNK_WIDTH / 2.0f),
                                                vec2(translation.x + CHUNK_WIDTH / 2.0f, translation.z + CHUNK_WIDTH / 2.0f),
                                                QUAD_TREE_DEPTH, HEIGHT_BIOME_DEPTH);

    m_noiseTexture = noiseData.NoiseTexture;

    BuildQuadTree(noiseData.MinMax, noiseData.HeightBiome);

    int quadTreesDivisionsCount = 1 << (QUAD_TREE_DEPTH - 1);
    m_drawZonesRanges = new vec4[quadTreesDivisionsCount * quadTreesDivisionsCount];

    for (int i = 0; i < quadTreesDivisionsCount; i++)
    {
        if (noiseData.MinMax[i])
        {
            delete[] noiseData.MinMax[i];
            noiseData.MinMax[i] = nullptr;
        }
    }

    if (noiseData.MinMax)
    {
        delete[] noiseData.MinMax;
        noiseData.MinMax = nullptr;
    }

    int heightBiomeDivisionsCount = 1 << (HEIGHT_BIOME_DEPTH - 1);
    
    for (int i = 0; i < heightBiomeDivisionsCount; i++)
    {
        if (noiseData.HeightBiome[i])
        {
            delete[] noiseData.HeightBiome[i];
            noiseData.HeightBiome[i] = nullptr;
        }
    }

    if (noiseData.HeightBiome)
    {
        delete[] noiseData.HeightBiome;
        noiseData.HeightBiome = nullptr;
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

    MathHelper::Frustum cameraFrustum = MathHelper::GetCameraFrustum(camera);

    FillZoneRanges(cameraFrustum, m_quadTree);
    UpdateZoneRangesBuffer();

    for (auto& folliageModel : m_folliageModelsInstances)
        m_folliageModelsInstances[folliageModel.first].clear();
    
    FillFolliageInstances(cameraFrustum, m_quadTree);
    
    for (auto& model : m_folliageModelsInstances)
    {
        sort(model.second.begin(), model.second.end(), [&](const mat4& a, const mat4& b)
            {
                const float* valsA = value_ptr(a);
                const float* valsB = value_ptr(b);

                return distance(vec3(valsA[12], valsA[13], valsA[14]), camera->GetPosition()) > distance(vec3(valsB[12], valsB[13], valsB[14]), camera->GetPosition());
            });
    }
}

void Chunk::DrawTerrain(Light* light, const vector<Material*>& terrainMaterials, Texture* terrainBiomesData)
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

void Chunk::DrawFolliage(Light* light)
{
    mat4 model      = translate(mat4(1.0f), GetTranslation());
    mat4 view       = m_camera->GetViewMatrix();
    mat4 projection = m_camera->GetProjectionMatrix();

    m_folliageShader->Use();

    m_folliageShader->SetMatrix4("View",           view);
    m_folliageShader->SetMatrix4("Projection",     projection);

    m_folliageShader->SetVec3("ChunkCenter",       GetTranslation());

    m_folliageShader->SetFloat("TerrainWidth",     CHUNK_WIDTH);
    m_folliageShader->SetFloat("GridWidth",        CHUNK_GRID_WIDTH);
    m_folliageShader->SetFloat("GridHeight",       CHUNK_GRID_HEIGHT);
    m_folliageShader->SetFloat("TerrainAmplitude", TERRAIN_AMPLITUDE);

    m_folliageShader->SetTexture("NoiseTexture",   m_noiseTexture, 0);

    m_folliageShader->SetVec4("AmbientColor",      light->GetAmbientColor());
    m_folliageShader->SetVec4("DiffuseColor",      light->GetDiffuseColor());
    m_folliageShader->SetVec3("LightDirection",    light->GetLightDirection());
    m_folliageShader->SetFloat("SpecularPower",    light->GetSpecularPower());
    m_folliageShader->SetVec3("CameraPosition",    m_camera->GetPosition());

    for (auto& model : m_folliageModelsInstances)
    {
        model.first->SetInstances(model.second);
        model.first->Draw(m_folliageShader, "DiffuseTextures", "NormalTextures", "SpecularTextures", 1);
    }
}

vec3 Chunk::GetTranslation() const
{
    return GetPositionForChunkId(m_chunkID);
}

vec3 Chunk::GetPositionForChunkId(Vec2Int chunkId)
{
    return vec3(chunkId.first * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS),
                0.0f,
                chunkId.second * (CHUNK_WIDTH - CHUNK_CLOSE_BIAS));
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

            vertices[i * verticesWidth + j].Position  = vec3(planePosition.x, 0.0f, planePosition.y);
            vertices[i * verticesWidth + j].TexCoords = vec2(j * TEX_COORDS_MULTIPLIER, verticesHeight - i * TEX_COORDS_MULTIPLIER - 1);
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

void Chunk::BuildQuadTree(PerlinNoise::MinMax** minMax, PerlinNoise::HeightBiome** heightBiome)
{
    m_quadTree = CreateNode(0, 
                            vec2(-CHUNK_WIDTH / 2.0f, -CHUNK_WIDTH / 2.0f), 
                            vec2(CHUNK_WIDTH / 2.0f, CHUNK_WIDTH / 2.0f), 
                            make_pair(0, 0),
                            minMax,
                            heightBiome);
}

Chunk::Node* Chunk::CreateNode(int depth, const vec2& bottomLeft, const vec2& topRight, pair<int, int> positionId, PerlinNoise::MinMax** minMax, PerlinNoise::HeightBiome** heightBiome)
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

        center  += FOLLIAGE_HEIGHT_BIAS / 2.0f;
        extents += FOLLIAGE_HEIGHT_BIAS / 2.0f;

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
                                         minMax,
                                         heightBiome);

        result->Children[1] = CreateNode(depth + 1,
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, bottomLeft.y),
                                         vec2(topRight.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         make_pair(1 + positionId.first * 2, positionId.second * 2),
                                         minMax,
                                         heightBiome);

        result->Children[2] = CreateNode(depth + 1,
                                         vec2(bottomLeft.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, topRight.y),
                                         make_pair(positionId.first * 2, positionId.second * 2 + 1),
                                         minMax,
                                         heightBiome);

        result->Children[3] = CreateNode(depth + 1,
                                         (topRight + bottomLeft) * 0.5f,
                                         topRight,
                                         make_pair(positionId.first * 2 + 1, positionId.second * 2 + 1),
                                         minMax,
                                         heightBiome);

        float maxAmplitude  = -TERRAIN_AMPLITUDE;
        float minAmplitude  = TERRAIN_AMPLITUDE;

        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
        {
            maxAmplitude = std::max(maxAmplitude, 
                                    result->Children[i]->BoundingBox.Center.y + result->Children[i]->BoundingBox.Extents.y);

            minAmplitude = std::min(minAmplitude, 
                                    result->Children[i]->BoundingBox.Center.y - result->Children[i]->BoundingBox.Extents.y);
        }

        float center       = (maxAmplitude + minAmplitude) / 2.0f;
        float extents      = (maxAmplitude - minAmplitude) / 2.0f;

        boundingBoxCenter  = vec3(boundingBoxCenter.x,  center,  boundingBoxCenter.z);
        boundingBoxExtents = vec3(boundingBoxExtents.x, extents, boundingBoxExtents.z);
    }

    result->BoundingBox = MathHelper::AABB(boundingBoxCenter, 
                                           boundingBoxExtents.x, boundingBoxExtents.y, boundingBoxExtents.z);

    if (result->IsLeaf)
    {
        int quadTreeWidth    = 1 << (QUAD_TREE_DEPTH    - 1);
        int heightBiomeWidth = 1 << (HEIGHT_BIOME_DEPTH - 1);

        int pixelsPerQuad    = heightBiomeWidth / quadTreeWidth;

        for (int x = 0; x < pixelsPerQuad; x++)
        {
            for (int y = 0; y < pixelsPerQuad; y++)
            {
                // TODO: properly do this (multiple noise maps, use one for this random)
                if (rand() % 40 != 0)
                    continue;

                PerlinNoise::HeightBiome crtHeightBiome = heightBiome[positionId.first * pixelsPerQuad + x][positionId.second * pixelsPerQuad + y];

                // TODO:
                // note that we can now calculate the height right here.
                // This change may help improve instancing performance (one call per world instead of chunk).
                auto translation = vec3(bottomLeft.x + (topRight.x - bottomLeft.x) * ((float)x / (float)(pixelsPerQuad - 1)), 
                                        0.0f, 
                                        bottomLeft.y + (topRight.y - bottomLeft.y) * ((float)y / (float)(pixelsPerQuad - 1))) +
                                   GetTranslation();

                translation = vec3(translation.x, 0.0f, translation.z);

                auto biomeModelsVectors = Biome::GetBiomeFolliageModels(crtHeightBiome.first, crtHeightBiome.second);

                if (!biomeModelsVectors.size())
                    continue;

                auto biomeModels = RouletteWheelSelection(biomeModelsVectors);

                if (!biomeModels.Models.size())
                    continue;

                auto biomeModel = RouletteWheelSelection(biomeModels.Models);

                auto model = translate(mat4(1.0f), translation) * 
                             scale(mat4(1.0f), vec3(biomeModel.Scale, biomeModel.Scale, biomeModel.Scale));

                if (result->DesiredInstances.find(biomeModel.Model) == result->DesiredInstances.end())
                    result->DesiredInstances[biomeModel.Model] = vector<mat4>();

                result->DesiredInstances[biomeModel.Model].push_back(model);
            }
        }
    }

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

void Chunk::FillFolliageInstances(const MathHelper::Frustum& frustum, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;

    if (node->IsLeaf)
    {
        for (auto& biomeModel : node->DesiredInstances)
        {
            if (m_folliageModelsInstances.find(biomeModel.first) == m_folliageModelsInstances.end())
                m_folliageModelsInstances[biomeModel.first] = vector<mat4>();

            for (auto& model : biomeModel.second)
                m_folliageModelsInstances[biomeModel.first].push_back(model);
        }
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            FillFolliageInstances(frustum, node->Children[i]);
    }
}

const Biome::FolliageModel& Chunk::RouletteWheelSelection(const std::vector<Biome::FolliageModel>& models)
{
    int resultIndex = 0;
    float totalSum = 0.0f;

    for (auto& model : models)
        totalSum += model.Chance;

    // TODO: replace this (multiple noise maps)
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float slice = r * totalSum;

    float accumulatedSum = 0.0f;

    int index = 0;
    for (auto& model : models)
    {
        accumulatedSum += model.Chance;
        if (accumulatedSum >= slice)
        {
            resultIndex = index;
            break;
        }

        index++;
    }

    return models[resultIndex];
}

const Biome::FolliageModelsVector& Chunk::RouletteWheelSelection(const std::vector<Biome::FolliageModelsVector>& models)
{
    int resultIndex = 0;
    float totalSum = 0.0f;

    for (auto& model : models)
        totalSum += model.Chance;

    // TODO: replace this (multiple noise maps)
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float slice = r * totalSum;

    float accumulatedSum = 0.0f;

    int index = 0;
    for (auto& model : models)
    {
        accumulatedSum += model.Chance;
        if (accumulatedSum >= slice)
        {
            resultIndex = index;
            break;
        }
        index++;
    }

    return models[resultIndex];
}