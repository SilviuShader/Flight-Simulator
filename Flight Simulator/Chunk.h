#pragma once

#include <set>
#include <utility>

#include <glm/glm.hpp>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "MathHelper.h"
#include "Model.h"
#include "Biome.h"

class Chunk
{
private:

    struct FolliageProperties
    {
        glm::vec3 Translation;
        float     Scale;
    };

    struct Node
    {
    public:

        static const int CHILDREN_COUNT = 4;

    public:

        Node();
        ~Node();

    public:

        Node*                                                                                               Children[CHILDREN_COUNT];
        bool                                                                                                IsLeaf;
        glm::vec4                                                                                           ZoneRange;
                                                                                                            
        Vec2Int                                                                                             PositionId;
                                                                                                            
        MathHelper::AABB                                                                                    BoundingBox;

        std::unordered_map<Biome::FolliageModel, std::vector<FolliageProperties>, Biome::HashFolliageModel> DesiredInstances;
    };

public:

    static const float CHUNK_WIDTH;
    static const float CHUNK_CLOSE_BIAS;

private:
           const float TERRAIN_AMPLITUDE     = 75.0f;
           const float FOLLIAGE_HEIGHT_BIAS  = 2.5f;

           const float DISTANCE_FOR_DETAILS  = 512.0f;
           const float MAX_TESSELATION       = 16.0f;
           const float TEX_COORDS_MULTIPLIER = 0.2f;

           const float GAMMA                 = 1.2f;

           const int   NOISE_TEXTURE_SIZE    = 1024;

    static const int   COLORS_COUNT          = 5;

    static const int   CHUNK_GRID_WIDTH      = 8;
    static const int   CHUNK_GRID_HEIGHT     = 8;

    static const int   QUAD_TREE_DEPTH       = 4;
    static const int   HEIGHT_BIOME_DEPTH    = 8;

    static const int   INDICES_COUNT         = CHUNK_GRID_WIDTH * CHUNK_GRID_HEIGHT * 6;

public:

    Chunk(PerlinNoise*, Shader*, std::pair<int, int>, Shader*, Shader*, Shader*);
    ~Chunk();

           void      Update(Camera*, float, bool);
           void      DrawTerrain(Light*, const std::vector<Material*>&, Texture*);
           void      DrawFolliage(Light*);

           glm::vec3 GetTranslation() const;

    static glm::vec3 GetPositionForChunkId(Vec2Int);


private:

                void  CreateTerrainBuffers();
                void  FreeTerrainBuffers();
                      
                void  BuildQuadTree(std::pair<float**, float**>, std::pair<float**, float**>, std::pair<float**, float**>);
                Node* CreateNode(int, const glm::vec2&, const glm::vec2&, std::pair<int, int>, std::pair<float**, float**>, std::pair<float**, float**>, std::pair<float**, float**>);
                      
                void  FillZoneRanges(const MathHelper::Frustum&, Node*);
                void  UpdateZoneRangesBuffer();
                      
                void  FillFolliageInstances(const MathHelper::Frustum&, Node*);

          template<typename T>
          const T&    RouletteWheelSelection(const std::vector<T>& models, float r)
          {
              int resultIndex = 0;
              float totalSum = 0.0f;

              for (auto& model : models)
                  totalSum += model.Chance;

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

private:

    Vec2Int                                                                          m_chunkID;
                                                                                     
    unsigned int                                                                     m_vbo;
    unsigned int                                                                     m_instanceVbo;
    unsigned int                                                                     m_ebo;
    unsigned int                                                                     m_vao;
                                                                                     
    Shader*                                                                          m_terrainShader;
    PerlinNoise*                                                                     m_perlinNoise;
    Texture*                                                                         m_heightTexture;
    Texture*                                                                         m_biomesTexture;
    glm::vec4*                                                                       m_drawZonesRanges; 

    std::unordered_map<std::pair<Model*, Shader*>, std::vector<glm::mat4>, HashPair> m_folliageModelsInstances;
                                                       
    int                                                                              m_zoneRangesIndex;
                                                                                     
    Node*                                                                            m_quadTree;
                                                                                     
    Camera*                                                                          m_camera;
    bool                                                                             m_renderDebug;
};