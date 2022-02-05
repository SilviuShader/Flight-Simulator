#pragma once

#include <glm/glm.hpp>
#include <utility>

#include "Shader.h"
#include "PerlinNoise.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "MathHelper.h"

class Chunk
{
private:

    struct Vertex 
    {
    public:

        Vertex();
        Vertex(glm::vec3, glm::vec2);

    public:

        glm::vec3 Position;
        glm::vec2 TexCoord;
    };

    struct Node
    {
    public:

        static const int CHILDREN_COUNT = 4;

    public:

        ~Node();

    public:

        Node*            Children[CHILDREN_COUNT];
        bool             IsLeaf;
        glm::vec2        BottomLeft;
        glm::vec2        TopRight;
        MathHelper::AABB BoundingBox;
    };

public:

    static const float CHUNK_WIDTH;
    static const float CHUNK_CLOSE_BIAS;

private:
           const float TERRAIN_AMPLITUDE     = 75.0f;

           const float DISTANCE_FOR_DETAILS  = 256.0f;
           const float MAX_TESSELATION       = 10.0f;
           const float TEX_COORDS_MULTIPLIER = 0.2f;

           const float GAMMA                 = 1.0f;

    static const int   COLORS_COUNT          = 5;

    static const int   CHUNK_GRID_WIDTH      = 10;
    static const int   CHUNK_GRID_HEIGHT     = 10;

    static const int   QUAD_TREE_DEPTH       = 4;

    static const int   INDICES_COUNT        = CHUNK_GRID_WIDTH * CHUNK_GRID_HEIGHT * 6;

public:

    Chunk(PerlinNoise*, Shader*, std::pair<int, int>);
    ~Chunk();

    void Draw(Light*, Camera*, const std::vector<Material*>&, Texture*);

private:

    void      CreateTerrainBuffers();
    void      FreeTerrainBuffers();

    void      BuildQuadTree();
    void      DrawNode(const MathHelper::Frustum&, Node*);

    glm::vec3 GetTranslation() const;

private:

    Node* CreateNode(int, const glm::vec2&, const glm::vec2&);

private:

    std::pair<int, int>    m_chunkID;

    unsigned int           m_vbo;
    unsigned int           m_ebo;
    unsigned int           m_vao;
                           
    Shader*                m_terrainShader;
    PerlinNoise*           m_perlinNoise;
    RenderTexture*         m_renderTexture;

    Node*                  m_quadTree;
};