#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include "glad/glad.h"

#include "Chunk.h"
#include "DebugHelper.h"
#include "VertexTypes.h"
#include "Biome.h"
#include "Terrain.h"
#include <glm/gtc/type_ptr.hpp>
#include "ShaderManager.h"

using namespace std;
using namespace glm;

const float Chunk::CHUNK_CLOSE_BIAS = 0.1f;

Chunk::Node::Node()
{
    memset(Children, 0, sizeof(Node*) * CHILDREN_COUNT);

    IsLeaf           = false;
    ZoneRange        = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    PositionId       = make_pair(0, 0);
    BoundingBox      = MathHelper::AABB();
    DesiredInstances = unordered_map<Biome::FolliageModel, vector<FolliageProperties>, Biome::HashFolliageModel>();
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

Chunk::Chunk(PerlinNoise* perlinNoise, pair<int, int> chunkID) :
    m_vbo(0),
    m_instanceVbo(0),
    m_ebo(0),
    m_vao(0),
    m_waterVbo(0),
    m_waterEbo(0),
    m_waterVao(0),
    m_perlinNoise(perlinNoise),
    m_chunkID(chunkID),
    m_quadTree(nullptr),
    m_renderDebug(false),
    m_waterTime(0.0f)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    CreateTerrainBuffers();
    CreateWaterBuffers();

    vec3 translation = GetTranslation();

    vec2 startPosition = vec2(translation.x - Terrain::CHUNK_WIDTH / 2.0f, translation.z - Terrain::CHUNK_WIDTH / 2.0f);
    vec2 endPosition   = vec2(translation.x + Terrain::CHUNK_WIDTH / 2.0f, translation.z + Terrain::CHUNK_WIDTH / 2.0f);

    PerlinNoise::NoiseParameters heightParameters;

    heightParameters.StartPosition = startPosition;
    heightParameters.EndPosition   = endPosition;
    heightParameters.Frequency     = Terrain::HEIGHT_FREQUENCY;
    heightParameters.FudgeFactor   = Terrain::HEIGHT_FUDGE_FACTOR;
    heightParameters.Exponent      = Terrain::HEIGHT_EXPONENT;
    heightParameters.OctavesCount  = Terrain::HEIGHT_OCTAVES_COUNT;
    heightParameters.TextureSize   = NOISE_TEXTURE_SIZE;
                                   
    m_heightTexture                = m_perlinNoise->RenderPerlinNoise(heightParameters);

    PerlinNoise::NoiseParameters biomeParameters;

            biomeParameters.StartPosition = startPosition;
            biomeParameters.EndPosition   = endPosition;
            biomeParameters.Frequency     = Terrain::BIOME_FREQUENCY;
            biomeParameters.FudgeFactor   = Terrain::BIOME_FUDGE_FACTOR;
            biomeParameters.Exponent      = Terrain::BIOME_EXPONENT;
            biomeParameters.OctavesCount  = Terrain::BIOME_OCTAVES_COUNT;
            biomeParameters.TextureSize   = NOISE_TEXTURE_SIZE;
            
            m_biomesTexture               = m_perlinNoise->RenderPerlinNoise(biomeParameters);
                                          
    float** minValues                     = m_heightTexture->GetDownscaleValues({ shaderManager->GetMinShader(),     4, 8}, QUAD_TREE_DEPTH);
    float** maxValues                     = m_heightTexture->GetDownscaleValues({ shaderManager->GetMaxShader(),     4, 8}, QUAD_TREE_DEPTH);
                                          
    float** heightValues                  = m_heightTexture->GetDownscaleValues({ shaderManager->GetAverageShader(), 4, 8}, HEIGHT_BIOME_DEPTH);
    float** biomeValues                   = m_biomesTexture->GetDownscaleValues({ shaderManager->GetAverageShader(), 4, 8 }, HEIGHT_BIOME_DEPTH);
                                          
    int     quadTreesDivisionsCount       = 1 << (QUAD_TREE_DEPTH - 1);
    int     heightBiomeDivisionsCount     = 1 << (HEIGHT_BIOME_DEPTH - 1);

    PerlinNoise::NoiseParameters folliageRandomnessParameters;

             folliageRandomnessParameters.StartPosition = startPosition;
             folliageRandomnessParameters.EndPosition   = endPosition;
             folliageRandomnessParameters.Frequency     = Terrain::FOLLIAGE_RANDOMNESS_FREQUENCY;
             folliageRandomnessParameters.FudgeFactor   = Terrain::FOLLIAGE_RANDOMNESS_FUDGE_FACTOR;
             folliageRandomnessParameters.Exponent      = Terrain::FOLLIAGE_RANDOMNESS_EXPONENT;
             folliageRandomnessParameters.OctavesCount  = Terrain::FOLLIAGE_RANDOMNESS_OCTAVES_COUNT;
             folliageRandomnessParameters.TextureSize   = heightBiomeDivisionsCount;

    Texture* folliageRandomnessMap                      = m_perlinNoise->RenderPerlinNoise(folliageRandomnessParameters);
    float**  folliageRandomnessValues                   = Texture::GetPixelsInfo(folliageRandomnessMap);

    PerlinNoise::NoiseParameters folliageSelectionRandomnessParameters;

             folliageSelectionRandomnessParameters.StartPosition = startPosition;
             folliageSelectionRandomnessParameters.EndPosition   = endPosition;
             folliageSelectionRandomnessParameters.Frequency     = Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FREQUENCY;
             folliageSelectionRandomnessParameters.FudgeFactor   = Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FUDGE_FACTOR;
             folliageSelectionRandomnessParameters.Exponent      = Terrain::FOLLIAGE_SELECTION_RANDOMNESS_EXPONENT;
             folliageSelectionRandomnessParameters.OctavesCount  = Terrain::FOLLIAGE_SELECTION_RANDOMNESS_OCTAVES_COUNT;
             folliageSelectionRandomnessParameters.TextureSize   = heightBiomeDivisionsCount;

    Texture* folliageSelectionRandomnessMap                      = m_perlinNoise->RenderPerlinNoise(folliageSelectionRandomnessParameters);
    float**  folliageSelectionRandomnessValues                   = Texture::GetPixelsInfo(folliageSelectionRandomnessMap);

    BuildQuadTree(make_pair(minValues, maxValues), make_pair(heightValues, biomeValues), make_pair(folliageRandomnessValues, folliageSelectionRandomnessValues));
    BuildWaterQuadTree();

    m_drawZonesRanges = new vec4[quadTreesDivisionsCount * quadTreesDivisionsCount];
    m_waterDrawZonesRanges = new vec4[quadTreesDivisionsCount * quadTreesDivisionsCount];

    for (int i = 0; i < heightBiomeDivisionsCount; i++)
    {
        if (folliageSelectionRandomnessValues[i])
        {
            delete[] folliageSelectionRandomnessValues[i];
            folliageSelectionRandomnessValues[i] = nullptr;
        }

        if (folliageRandomnessValues[i])
        {
            delete[] folliageRandomnessValues[i];
            folliageRandomnessValues[i] = nullptr;
        }
    }

    if (folliageSelectionRandomnessValues)
    {
        delete[] folliageSelectionRandomnessValues;
        folliageSelectionRandomnessValues = nullptr;
    }

    if (folliageRandomnessValues)
    {
        delete[] folliageRandomnessValues;
        folliageRandomnessValues = nullptr;
    }

    if (folliageSelectionRandomnessMap)
    {
        delete folliageSelectionRandomnessMap;
        folliageSelectionRandomnessMap = nullptr;
    }

    if (folliageRandomnessMap)
    {
        delete folliageRandomnessMap;
        folliageRandomnessMap = nullptr;
    }

    for (int i = 0; i < quadTreesDivisionsCount; i++)
    {
        if (maxValues[i])
        {
            delete[] maxValues[i];
            maxValues[i] = nullptr;
        }

        if (minValues[i])
        {
            delete[] minValues[i];
            minValues[i] = nullptr;
        }
    }

    if (maxValues)
    {
        delete[] maxValues;
        maxValues = nullptr;
    }

    if (minValues)
    {
        delete[] minValues;
        minValues = nullptr;
    }

    for (int i = 0; i < heightBiomeDivisionsCount; i++)
    {
        if (biomeValues[i])
        {
            delete[] biomeValues[i];
            biomeValues[i] = nullptr;
        }

        if (heightValues[i])
        {
            delete[] heightValues[i];
            heightValues[i] = nullptr;
        }
    }

    if (biomeValues)
    {
        delete[] biomeValues;
        biomeValues = nullptr;
    }

    if (heightValues)
    {
        delete[] heightValues;
        heightValues = nullptr;
    }
}

Chunk::~Chunk()
{
    if (m_waterDrawZonesRanges)
    {
        delete[] m_waterDrawZonesRanges;
        m_waterDrawZonesRanges = nullptr;
    }

    if (m_drawZonesRanges)
    {
        delete[] m_drawZonesRanges;
        m_drawZonesRanges = nullptr;
    }

    if (m_waterQuadTree)
    {
        delete m_waterQuadTree;
        m_waterQuadTree = nullptr;
    }

    if (m_quadTree)
    {
        delete m_quadTree;
        m_quadTree = nullptr;
    }

    if (m_biomesTexture)
    {
        delete m_biomesTexture;
        m_biomesTexture = nullptr;
    }

    if (m_heightTexture)
    {
        delete m_heightTexture;
        m_heightTexture = nullptr;
    }

    FreeWaterBuffers();
    FreeTerrainBuffers();
}

void Chunk::Update(Camera* camera, float deltaTime, bool renderDebug)
{
    m_renderDebug = renderDebug;

    m_zoneRangesIndex = 0;

    MathHelper::Frustum cameraFrustum = MathHelper::GetCameraFrustum(camera);

    FillZoneRanges(cameraFrustum, m_quadTree);
    UpdateZoneRangesBuffer();

    for (auto& folliageModel : m_folliageModelsInstances)
        m_folliageModelsInstances[folliageModel.first].clear();
    
    FillFolliageInstances(camera, cameraFrustum, m_quadTree);
    
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

void Chunk::UpdateWater(Camera* camera, float deltaTime, bool renderDebug)
{
    m_renderDebug = renderDebug;

    m_waterTime += deltaTime;

    m_waterZoneRangesIndex = 0;

    MathHelper::Frustum cameraFrustum = MathHelper::GetCameraFrustum(camera);

    FillWaterZoneRanges(cameraFrustum, m_waterQuadTree);
    UpdateWaterZoneRangesBuffer();
}

void Chunk::DrawTerrain(Camera* camera, Light* light, const vector<Material*>& terrainMaterials, Texture* terrainBiomesData)
{
    vec3           cameraPosition = camera->GetPosition();
                   
    mat4           model          = translate(mat4(1.0f), GetTranslation());
    mat4           view           = camera->GetViewMatrix();
    mat4           projection     = camera->GetProjectionMatrix();

    ShaderManager* shaderManager  = ShaderManager::GetInstance();
    Shader*        terrainShader  = shaderManager->GetTerrainShader();

    terrainShader->Use();

    terrainShader->SetMatrix4("Model",                     model);

    terrainShader->SetVec3("CameraPosition",               cameraPosition);
    terrainShader->SetFloat("DistanceForDetails",          Terrain::DISTANCE_FOR_DETAILS);
    terrainShader->SetFloat("TessellationLevel",           Terrain::MAX_TESSELATION);

    terrainShader->SetTexture("HeightTexture",             m_heightTexture, 0);
    terrainShader->SetTexture("BiomeTexture",              m_biomesTexture, 1);

    terrainShader->SetMatrix4("View",                      view);
    terrainShader->SetMatrix4("Projection",                projection);

    terrainShader->SetFloat("TerrainWidth",                Terrain::CHUNK_WIDTH);
    terrainShader->SetFloat("GridWidth",                   CHUNK_GRID_WIDTH);
    terrainShader->SetFloat("GridHeight",                  CHUNK_GRID_HEIGHT);
    terrainShader->SetFloat("TerrainAmplitude",            Terrain::TERRAIN_AMPLITUDE);

    terrainShader->SetFloat("Gamma",                       Terrain::GAMMA);

    terrainShader->SetLight(camera, light);

    terrainShader->SetInt("BiomesCount",                   terrainBiomesData->GetWidth());
    terrainShader->SetInt("MaterialsPerBiome",             terrainBiomesData->GetHeight());

    terrainShader->SetTexture("BiomeMaterialsTexture",     terrainBiomesData, 2);

    terrainShader->SetMaterials("TerrainTextures",
                                "TerrainNormalTextures", 
                                "TerrainSpecularTextures", terrainMaterials,  3);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElementsInstanced(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0, m_zoneRangesIndex);
}

void Chunk::DrawFolliage(Camera* camera, Light* light)
{
    mat4 model      = translate(mat4(1.0f), GetTranslation());
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = camera->GetProjectionMatrix();

    for (auto& keyValue : m_folliageModelsInstances)
    {
        auto modelShader = keyValue.first;
        auto model = modelShader.first;
        auto shader = modelShader.second;

        shader->Use();

        shader->SetMatrix4("View",           view);
        shader->SetMatrix4("Projection",     projection);

        shader->SetVec3("ChunkCenter",       GetTranslation());

        shader->SetFloat("TerrainWidth",     Terrain::CHUNK_WIDTH);
        shader->SetFloat("GridWidth",        CHUNK_GRID_WIDTH);
        shader->SetFloat("GridHeight",       CHUNK_GRID_HEIGHT);
        shader->SetFloat("TerrainAmplitude", Terrain::TERRAIN_AMPLITUDE);

        shader->SetTexture("NoiseTexture",   m_heightTexture, 0);

        if (shader->HasLightUniforms())
            shader->SetLight(camera, light);

        model->SetInstances(keyValue.second);
        model->Draw(shader, "DiffuseTextures", "NormalTextures", "SpecularTextures", 1);
    }
}

void Chunk::DrawWater(Camera* camera, Light* light, Texture* refractionTexture, Texture* reflectionTexture, Texture* refractionDepthTexture, Texture* reflectionDepthTexture, float waterMoveFactor, Material* waterMaterial)
{
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    Shader*        waterShader   = shaderManager->GetWaterShader();

    mat4 model      = translate(mat4(1.0f), GetTranslation() + vec3(0.0f, Terrain::WATER_LEVEL, 0.0f)); 
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = camera->GetProjectionMatrix();

    // TODO: Find a way to pass all these variabiles without hard-coded constants.

    waterShader->Use();

    waterShader->SetMatrix4("Model",                      model);
    waterShader->SetMatrix4("View",                       view);
    waterShader->SetMatrix4("Projection",                 projection);
                                                          
    waterShader->SetVec3("CameraPosition",                camera->GetPosition());
                                                          
    waterShader->SetFloat("Tiling",                       100.0f);
                                                          
    waterShader->SetFloat("DistanceForDetails",           100.0f);
    waterShader->SetFloat("TessellationLevel",            2);
                                                          
    waterShader->SetVec4("WavesWeights",                  vec4(4, 3, 2, 1));
    waterShader->SetVec4("WavesSpeeds",                   vec4(0.125f, 0.25f, 0.5f, 1.0f));
    waterShader->SetVec4("WavesOffsets",                  vec4(0.2f, 0.4f, 0.8f, 1.6f));
    waterShader->SetVec4("WavesRadiuses",                 vec4(2.0f, 1.0f, 0.5f, 0.25f));
                                                          
    waterShader->SetVec2("WaveADirection",                vec2(1.0f, 0.0f));
    waterShader->SetVec2("WaveBDirection",                vec2(0.0f, 1.0f));
    waterShader->SetVec2("WaveCDirection",                vec2(-1.0f, 0.0f));
    waterShader->SetVec2("WaveDDirection",                vec2(0.0f, -1.0f));
                                                          
    waterShader->SetFloat("Time",                         m_waterTime);
    waterShader->SetFloat("ScreenEdgeCorrectionDistance", 0.2f);

    waterShader->SetTexture("RefractionTexture",          refractionTexture,      0);
    waterShader->SetTexture("ReflectionTexture",          reflectionTexture,      1);
    waterShader->SetTexture("RefractionDepthTexture",     refractionDepthTexture, 2);
    waterShader->SetTexture("ReflectionDepthTexture",     reflectionDepthTexture, 3);
    waterShader->SetMaterials("WaterTextures",
                              "WaterNormalTextures",
                              "WaterSpecularTextures", { waterMaterial }, 4);
                                                          
    waterShader->SetFloat("FadeWaterDepth",               3.0f);
                                                          
    waterShader->SetFloat("ReflectivePower",              0.5);
    waterShader->SetFloat("TextureMultiplier",            0.5f);
                                                          
    waterShader->SetLight(camera, light);                 
                                                          
    waterShader->SetFloat("Near",                         camera->GetNear());
    waterShader->SetFloat("Far",                          camera->GetFar());

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(m_waterVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_waterEbo);
    glDrawElementsInstanced(GL_PATCHES, INDICES_COUNT, GL_UNSIGNED_INT, 0, m_waterZoneRangesIndex);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

vec3 Chunk::GetTranslation() const
{
    return GetPositionForChunkId(m_chunkID);
}

vec3 Chunk::GetPositionForChunkId(Vec2Int chunkId)
{
    return vec3(chunkId.first  * (Terrain::CHUNK_WIDTH - CHUNK_CLOSE_BIAS),
                0.0f,
                chunkId.second * (Terrain::CHUNK_WIDTH - CHUNK_CLOSE_BIAS));
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

void Chunk::CreateWaterBuffers()
{
    // TODO: Implement instancing for water and use the same function for generating the chunk geometry, or use the same geometry
    // TODO: Also make water related to the biomes somehow.
    constexpr int verticesWidth = CHUNK_GRID_WIDTH + 1;
    constexpr int verticesHeight = CHUNK_GRID_HEIGHT + 1;

    constexpr int verticesCount = verticesWidth * verticesHeight;

    VertexPositionTexture* vertices = new VertexPositionTexture[verticesCount];

    for (int i = 0; i < verticesHeight; i++)
    {
        for (int j = 0; j < verticesWidth; j++)
        {
            float adjustedI = (float)i / (float)(verticesWidth - 1);
            float adjustedJ = (float)j / (float)(verticesHeight - 1);

            vec2 planePosition = vec2(adjustedJ, adjustedI);

            vertices[i * verticesWidth + j].Position = vec3(planePosition.x, 0.0f, planePosition.y);
            vertices[i * verticesWidth + j].TexCoords = vec2(j * TEX_COORDS_MULTIPLIER, verticesHeight - i * TEX_COORDS_MULTIPLIER - 1);
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

    glGenVertexArrays(1, &m_waterVao);
    
    glBindVertexArray(m_waterVao);

    glGenBuffers(1, &m_waterVbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_waterVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPositionTexture) * verticesCount, vertices, GL_STATIC_DRAW);

    VertexPositionTexture::SetLayout();

    glGenBuffers(1, &m_waterInstanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_waterInstanceVbo);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

    glGenBuffers(1, &m_waterEbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_waterEbo);
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

void Chunk::FreeWaterBuffers()
{
    glBindVertexArray(m_waterVao);

    VertexPositionTexture::ResetLayout();
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_waterVbo);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_waterInstanceVbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_waterEbo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_waterVao);
}

void Chunk::BuildQuadTree(pair<float**, float**> minMax, pair<float**, float**> heightBiome, pair<float**, float**> folliageRandomnessValues)
{
    m_quadTree = CreateNode(0, 
                            vec2(-Terrain::CHUNK_WIDTH / 2.0f, -Terrain::CHUNK_WIDTH / 2.0f),
                            vec2( Terrain::CHUNK_WIDTH / 2.0f,  Terrain::CHUNK_WIDTH / 2.0f),
                            make_pair(0, 0),
                            minMax,
                            heightBiome,
                            folliageRandomnessValues);
}

Chunk::Node* Chunk::CreateNode(int depth, const vec2& bottomLeft, const vec2& topRight, pair<int, int> positionId, pair<float**, float**> minMax, pair<float**, float**> heightBiome, pair<float**, float**> folliageRandomnessValues)
{
    if (depth >= QUAD_TREE_DEPTH)
        return nullptr;

    Node* result            = new Node();

    result->ZoneRange       = vec4(bottomLeft.x, bottomLeft.y, topRight.x, topRight.y);
    result->PositionId      = positionId;
    result->LinkedNode      = nullptr;

    vec3 boundingBoxCenter  = vec3((bottomLeft.x + topRight.x) * 0.5f, 0.0f,
                                   (bottomLeft.y + topRight.y) * 0.5f) + GetTranslation();

    vec3 boundingBoxExtents = vec3((topRight.x - bottomLeft.x) * 0.5f, Terrain::TERRAIN_AMPLITUDE, 
                                   (topRight.y - bottomLeft.y) * 0.5f);

    if (depth == QUAD_TREE_DEPTH - 1)
    {
        result->IsLeaf = true;

        float minAmplitude = minMax.first [positionId.first][positionId.second] * Terrain::TERRAIN_AMPLITUDE;
        float maxAmplitude = minMax.second[positionId.first][positionId.second] * Terrain::TERRAIN_AMPLITUDE;

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
                                         heightBiome, 
                                         folliageRandomnessValues);

        result->Children[1] = CreateNode(depth + 1,
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, bottomLeft.y),
                                         vec2(topRight.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         make_pair(1 + positionId.first * 2, positionId.second * 2),
                                         minMax,
                                         heightBiome, 
                                         folliageRandomnessValues);

        result->Children[2] = CreateNode(depth + 1,
                                         vec2(bottomLeft.x, (bottomLeft.y + topRight.y) * 0.5f),
                                         vec2((bottomLeft.x + topRight.x) * 0.5f, topRight.y),
                                         make_pair(positionId.first * 2, positionId.second * 2 + 1),
                                         minMax,
                                         heightBiome,
                                         folliageRandomnessValues);

        result->Children[3] = CreateNode(depth + 1,
                                         (topRight + bottomLeft) * 0.5f,
                                         topRight,
                                         make_pair(positionId.first * 2 + 1, positionId.second * 2 + 1),
                                         minMax,
                                         heightBiome,
                                         folliageRandomnessValues);

        float maxAmplitude  = -Terrain::TERRAIN_AMPLITUDE;
        float minAmplitude  =  Terrain::TERRAIN_AMPLITUDE;

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
                int xIndex = positionId.first  * pixelsPerQuad + x;
                int yIndex = positionId.second * pixelsPerQuad + y;

                if (folliageRandomnessValues.first[xIndex][yIndex] < Terrain::FOLLIAGE_RANDOMNESS_THRESHOLD)
                    continue;

                float height = heightBiome.first [xIndex][yIndex];
                float biome  = heightBiome.second[xIndex][yIndex];
                
                auto translation = vec3(bottomLeft.x + (topRight.x - bottomLeft.x) * ((float)x / (float)(pixelsPerQuad - 1)), 
                                        0.0f, 
                                        bottomLeft.y + (topRight.y - bottomLeft.y) * ((float)y / (float)(pixelsPerQuad - 1))) +
                                   GetTranslation();

                translation = vec3(translation.x, 0.0f, translation.z);

                auto biomeModelsVectors = Biome::GetBiomeFolliageModels(height, biome);

                if (!biomeModelsVectors.size())
                    continue;

                auto biomeModels = RouletteWheelSelection(biomeModelsVectors, folliageRandomnessValues.second[xIndex][yIndex]);

                if (!biomeModels.Models.size())
                    continue;

                auto biomeModel = RouletteWheelSelection(biomeModels.Models, folliageRandomnessValues.second[yIndex][xIndex]); // little "hack" so we don't need two separate maps

                if (result->DesiredInstances.find(biomeModel) == result->DesiredInstances.end())
                    result->DesiredInstances[biomeModel] = vector<FolliageProperties>();

                result->DesiredInstances[biomeModel].push_back( {translation, biomeModels.Chance });
            }
        }
    }

    return result;
}

void Chunk::BuildWaterQuadTree()
{
    m_waterQuadTree = CreateWaterNode(0, m_quadTree);
}

Chunk::Node* Chunk::CreateWaterNode(int depth, Node* linkedNode)
{
    if (depth >= QUAD_TREE_DEPTH)
        return nullptr;

    Node* result = new Node();

    MathHelper::AABB linkedBoundingBox = linkedNode->BoundingBox;

    result->ZoneRange  = linkedNode->ZoneRange;
    result->PositionId = linkedNode->PositionId;
    result->LinkedNode = linkedNode;
    result->IsLeaf     = linkedNode->IsLeaf;

    
    if (!result->IsLeaf)
    {
        result->Children[0] = CreateWaterNode(depth + 1, linkedNode->Children[0]);
        result->Children[1] = CreateWaterNode(depth + 1, linkedNode->Children[1]);
        result->Children[2] = CreateWaterNode(depth + 1, linkedNode->Children[2]);
        result->Children[3] = CreateWaterNode(depth + 1, linkedNode->Children[3]);
    }

    MathHelper::AABB aabb = linkedBoundingBox;
    aabb.Center = vec3(aabb.Center.x, Terrain::WATER_LEVEL, aabb.Center.z);
    aabb.Extents = vec3(aabb.Extents.x, 3.0f, aabb.Extents.z); // TODO: Replace this hard-coded number
    result->BoundingBox = aabb;

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
            DebugHelper::GetInstance()->AddRectangleInstance(node->BoundingBox.Center, node->BoundingBox.Extents);
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

void Chunk::FillWaterZoneRanges(const MathHelper::Frustum& frustum, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;

    MathHelper::AABB otherAABB = node->LinkedNode->BoundingBox;
    MathHelper::AABB currentAABB = node->BoundingBox;

    if (otherAABB.Center.y - otherAABB.Extents.y > currentAABB.Center.y + currentAABB.Extents.y)
        return;

    if (node->IsLeaf)
    {
        m_waterDrawZonesRanges[m_waterZoneRangesIndex++] = node->ZoneRange;
        if (m_renderDebug)
            DebugHelper::GetInstance()->AddRectangleInstance(node->BoundingBox.Center, node->BoundingBox.Extents);
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            FillWaterZoneRanges(frustum, node->Children[i]);
    }
}

void Chunk::UpdateWaterZoneRangesBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_waterInstanceVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * m_waterZoneRangesIndex, m_waterDrawZonesRanges, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Chunk::FillFolliageInstances(Camera* camera, const MathHelper::Frustum& frustum, Node* node)
{
    if (!node->BoundingBox.IsOnFrustum(frustum))
        return;

    if (node->IsLeaf)
    {
        for (auto& biomeModel : node->DesiredInstances)
        {
            vec3 cameraPosition = camera->GetPosition();

            for (auto& folliageProperties : biomeModel.second)
            {
                vec3    translation    = folliageProperties.Translation;
                mat4    modelMatrix    = identity<mat4>();

                float   scaleMultip    = folliageProperties.Scale;
                                       
                float   dist           = distance(vec2(translation.x,    translation.z), 
                                                  vec2(cameraPosition.x, cameraPosition.z));

                float   distPercentage = dist / camera->GetFar();

                auto&   modelLODs      = biomeModel.first.ModelLODs;
                int     lodsCount      = modelLODs.size();

                int     power          = MathHelper::PowerCeil(lodsCount);
                int     lodIndex       = power;
                bool    foundLOD       = false;

                for (int indexSubstract = power; indexSubstract > 0; indexSubstract >>= 1)
                {
                    int newIndex = lodIndex - indexSubstract;
                    if (newIndex >= 0 && newIndex < lodsCount)
                    {
                        if (modelLODs[newIndex].MaxDistance >= distPercentage)
                        {
                            lodIndex = newIndex;
                            foundLOD = true;
                        }
                    }
                }

                if (!foundLOD)
                    continue;

                auto& lod          = modelLODs[lodIndex];
                vec3  toCamera     = cameraPosition - translation;
                                   
                float angle        = lod.Bilboarded ? atan2(toCamera.x, toCamera.z) : 0.0f;
                mat4  rotation     = rotate(mat4(1.0f), angle, vec3(0.0f, 1.0f, 0.0f));

                float scaleFactor = lod.Scale * scaleMultip;

                      modelMatrix = translate(mat4(1.0f), translation) * 
                                    scale(mat4(1.0f), vec3(scaleFactor, scaleFactor, scaleFactor)) * rotation;

                auto  mapKey      = make_pair(lod.Model, lod.Shader);

                if (m_folliageModelsInstances.find(mapKey) == m_folliageModelsInstances.end())
                    m_folliageModelsInstances[mapKey] = vector<mat4>();

                m_folliageModelsInstances[mapKey].push_back(modelMatrix);
            }
        }
    }
    else
    {
        for (int i = 0; i < Node::CHILDREN_COUNT; i++)
            FillFolliageInstances(camera, frustum, node->Children[i]);
    }
}