#include "Terrain.h"

#include <queue>
#include "ShaderManager.h"

using namespace std;
using namespace glm;

const float Terrain::CHUNK_WIDTH                                 = 64.0f;
const float Terrain::TERRAIN_AMPLITUDE                           = 75.0f;
const float Terrain::DISTANCE_FOR_DETAILS                        = 512.0f;
const float Terrain::MAX_TESSELATION                             = 16.0f;
const float Terrain::GAMMA                                       = 1.5f;

const float Terrain::HEIGHT_FREQUENCY                            = 0.025f;
const float Terrain::HEIGHT_FUDGE_FACTOR                         = 1.2f;
const float Terrain::HEIGHT_EXPONENT                             = 4.0f;
const int   Terrain::HEIGHT_OCTAVES_COUNT                        = 20;
										                         
const float Terrain::BIOME_FREQUENCY                             = 0.01f;
const float Terrain::BIOME_FUDGE_FACTOR                          = 1.0f;
const float Terrain::BIOME_EXPONENT                              = 1.0f;
const int   Terrain::BIOME_OCTAVES_COUNT                         = 10;
													             
const float Terrain::FOLLIAGE_RANDOMNESS_FREQUENCY               = 1.0f;
const float Terrain::FOLLIAGE_RANDOMNESS_FUDGE_FACTOR            = 1.0f;
const float Terrain::FOLLIAGE_RANDOMNESS_EXPONENT                = 1.0f;
const int   Terrain::FOLLIAGE_RANDOMNESS_OCTAVES_COUNT           = 20;
													             
const float Terrain::FOLLIAGE_RANDOMNESS_THRESHOLD               = 0.6f;

const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FREQUENCY     = 0.9f;
const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FUDGE_FACTOR  = 1.2f;
const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_EXPONENT      = 3.0f;
const int   Terrain::FOLLIAGE_SELECTION_RANDOMNESS_OCTAVES_COUNT = 20;

Terrain::Terrain() : 
	m_accumulatedCurrentChunksTime(0.0f),
	m_firstFrame(true)
{
	CreateTerrainObjects();
}

Terrain::~Terrain()
{
	for (auto& keyVal : m_chunks)
	{
		if (keyVal.second)
		{
			delete keyVal.second;
			keyVal.second = nullptr;
		}
	}

	m_chunks.clear();

	FreeTerrainObjects();
}

void Terrain::Udpate(Camera* camera, float deltaTime, bool renderDebug)
{
	UpdateCurrentChunks(camera, deltaTime);

	for (auto& chunk : m_chunksList)
		chunk->Update(camera, deltaTime, renderDebug);

	m_firstFrame = false;
}

void Terrain::Draw(Camera* camera, Light* light)
{
	for (auto& chunk : m_chunksList)
		chunk->DrawTerrain(camera, light, m_terrainMaterials, m_terrainBiomesData);

	for (auto& chunk : m_chunksList)
		chunk->DrawFolliage(camera, light);
}

void Terrain::CreateTerrainObjects()
{
	          m_noise                    = new PerlinNoise();

	Material* snow2                      = new Material("Assets/snow_02_diff_1k.png",             "Assets/snow_02_nor_gl_1k.png",             "Assets/snow_02_spec_1k.png");
	Material* medievalBlocks             = new Material("Assets/medieval_blocks_02_diff_1k.png",  "Assets/medieval_blocks_02_nor_gl_1k.png",  "Assets/medieval_blocks_02_spec_1k.png");
	Material* brownMudLeaves             = new Material("Assets/brown_mud_leaves_01_diff_1k.png", "Assets/brown_mud_leaves_01_nor_gl_1k.png", "Assets/brown_mud_leaves_01_spec_1k.png");
	Material* forestLeaves               = new Material("Assets/forest_leaves_03_diff_1k.png",    "Assets/forest_leaves_03_nor_gl_1k.png");
	Material* snowFieldAerial            = new Material("Assets/snow_field_aerial_col_1k.png",    "Assets/snow_field_aerial_nor_gl_1k.png");
	Material* snow3                      = new Material("Assets/snow_03_diff_1k.png",             "Assets/snow_03_nor_gl_1k.png",             "Assets/snow_03_spec_1k.png");

	Biome* iceBiome = Biome::CreateBiome();

	iceBiome->AddTerrainLevel(snowFieldAerial);
	iceBiome->AddTerrainLevel(snow3);
	iceBiome->AddTerrainLevel(snow2);
	iceBiome->AddTerrainLevel(snow2);

	Biome* forestBiome = Biome::CreateBiome();

	ShaderManager* shaderManager = ShaderManager::GetInstance();

	Biome::FolliageModel grassModel = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/grass.obj",         true), shaderManager->GetFolliageShader(),           0.05f, 0.05f),
			Biome::ModelLevelOfDetail(new Model("Assets/Models/GrassBilboard.png", true), shaderManager->GetFolliageBilboardedShader(), 5.0f,   1.0f, true)
		}, 1.0f);

	forestBiome->AddTerrainLevel(forestLeaves, { grassModel });
	forestBiome->AddTerrainLevel(brownMudLeaves);
	forestBiome->AddTerrainLevel(medievalBlocks);
	forestBiome->AddTerrainLevel(snow3);

	m_terrainBiomesData = Biome::CreateBiomesTexture();
	m_terrainMaterials  = Biome::GetBiomesMaterials();
}

void Terrain::FreeTerrainObjects()
{
	Biome::Free();

	if (m_noise)
	{
		delete m_noise;
		m_noise = nullptr;
	}
}

void Terrain::UpdateChunksVisibility(Camera* camera, float deltaTime, int diffSize)
{
	vec3                                         cameraPos         = camera->GetPosition();
	vec3                                         cameraChunkOrigin = cameraPos - vec3((CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS) / 2.0f,
                                                                                      0.0f,
                                                                                      (CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS) / 2.0f);
									             
	Vec2Int                                      currentID         = make_pair(int(cameraChunkOrigin.x / (CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)),
		                                                                       int(cameraChunkOrigin.z / (CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)));
									             
	queue<Vec2Int>                               exploreChunksQueue;
	unordered_set<Vec2Int, HashHelper::HashPair> targetChunksSet;

	exploreChunksQueue.push(currentID);
	targetChunksSet.insert(currentID);

	int dx[] = { -1, 0, 1, 0 };
	int dy[] = { 0, -1, 0, 1 };
	int directionsCount = sizeof(dx) / sizeof(int);

	while (targetChunksSet.size() < MAX_CHUNKS)
	{
		Vec2Int currentChunk = exploreChunksQueue.front();
		exploreChunksQueue.pop();

		for (int i = 0; i < directionsCount; i++)
		{
			if (targetChunksSet.size() >= MAX_CHUNKS)
				break;

			Vec2Int neighbour = make_pair(currentChunk.first + dx[i],
				                          currentChunk.second + dy[i]);

			if (targetChunksSet.find(neighbour) == targetChunksSet.end())
			{
				targetChunksSet.insert(neighbour);
				exploreChunksQueue.push(neighbour);
			}
		}
	}

	vector<Vec2Int> toErase;

	sort(toErase.begin(), toErase.end(), [&](Vec2Int a, Vec2Int b)
		{
			float distA = abs(currentID.first - a.first) + abs(currentID.second - a.second);
			float distB = abs(currentID.first - b.first) + abs(currentID.second - b.second);
			return distB < distA;
		});

	for (auto& keyVal : m_chunks)
	{
		if (targetChunksSet.find(keyVal.first) == targetChunksSet.end())
		{
			if (toErase.size() < diffSize)
			{
				delete keyVal.second;
				keyVal.second = nullptr;

				toErase.push_back(keyVal.first);
			}
		}
	}

	for (auto& id : toErase)
		m_chunks.erase(id);


	vector<Vec2Int> targetChunks;
	for (auto& targetChunk : targetChunksSet)
		targetChunks.push_back(targetChunk);

	sort(targetChunks.begin(), targetChunks.end(), [&](Vec2Int a, Vec2Int b)
		{
			float distA = abs(currentID.first - a.first) + abs(currentID.second - a.second);
			float distB = abs(currentID.first - b.first) + abs(currentID.second - b.second);
			return distA < distB;
		});

	int additions = 0;

	for (auto& targetChunk : targetChunks)
	{
		if (m_chunks.find(targetChunk) == m_chunks.end())
		{
			if (additions < diffSize)
			{
				Chunk* chunk = new Chunk(m_noise, targetChunk);
				m_chunks[targetChunk] = chunk;

				additions++;
			}
		}
	}
}

void Terrain::UpdateCurrentChunks(Camera* camera, float deltaTime)
{
	m_accumulatedCurrentChunksTime += deltaTime;

	if (m_accumulatedCurrentChunksTime < TIME_TO_UPDATE_CURRENT_CHUNKS && !m_firstFrame)
		return;

	m_accumulatedCurrentChunksTime -= m_firstFrame ? 0.0f : TIME_TO_UPDATE_CURRENT_CHUNKS;

	UpdateChunksVisibility(camera, deltaTime, m_firstFrame ? MAX_CHUNKS : 1);
	m_chunksList.clear();

	for (auto& keyVal : m_chunks)
		m_chunksList.push_back(keyVal.second);

	sort(m_chunksList.begin(), m_chunksList.end(), [&](Chunk* a, Chunk* b)
		{
			return distance(a->GetTranslation(), camera->GetPosition()) > distance(b->GetTranslation(), camera->GetPosition());
		});
}
