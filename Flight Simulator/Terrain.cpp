#include "Terrain.h"

#include <queue>
#include "ShaderManager.h"

using namespace std;
using namespace glm;

const float Terrain::CHUNK_WIDTH                                 = 64.0f;
const float Terrain::TERRAIN_AMPLITUDE                           = 100.0f;
const float Terrain::WATER_LEVEL                                 = 10.0f;
const float Terrain::DISTANCE_FOR_DETAILS                        = 256.0f;
const float Terrain::MAX_TESSELATION                             = 8.0f;
const float Terrain::GAMMA                                       = 1.0f;

const float Terrain::HEIGHT_FREQUENCY                            = 0.0125f;
const float Terrain::HEIGHT_FUDGE_FACTOR                         = 1.2f;
const float Terrain::HEIGHT_EXPONENT                             = 4.0f;
const int   Terrain::HEIGHT_OCTAVES_COUNT                        = 20;
										                         
const float Terrain::BIOME_FREQUENCY                             = 0.005f;
const float Terrain::BIOME_FUDGE_FACTOR                          = 1.0f;
const float Terrain::BIOME_EXPONENT                              = 1.0f;
const int   Terrain::BIOME_OCTAVES_COUNT                         = 10;
													             
const float Terrain::FOLLIAGE_RANDOMNESS_FREQUENCY               = 0.5f;
const float Terrain::FOLLIAGE_RANDOMNESS_FUDGE_FACTOR            = 1.0f;
const float Terrain::FOLLIAGE_RANDOMNESS_EXPONENT                = 1.0f;
const int   Terrain::FOLLIAGE_RANDOMNESS_OCTAVES_COUNT           = 20;
													             
const float Terrain::FOLLIAGE_RANDOMNESS_THRESHOLD               = 0.6f;

const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FREQUENCY     = 0.9f;
const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_FUDGE_FACTOR  = 1.5f;
const float Terrain::FOLLIAGE_SELECTION_RANDOMNESS_EXPONENT      = 4.0f;
const int   Terrain::FOLLIAGE_SELECTION_RANDOMNESS_OCTAVES_COUNT = 20;

const float Terrain::WATER_MOVE_SPEED                            = 0.01f;

Terrain::Terrain() : 
	m_accumulatedCurrentChunksTime(0.0f),
	m_firstFrame(true),
	m_waterMoveFactor(0.0f),
	m_waterTime(0.0f)
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

void Terrain::Udpate(Camera* camera, float deltaTime, bool renderDebug, bool renderFoliage)
{
	UpdateCurrentChunks(camera, deltaTime);

	for (auto& chunk : m_chunksList)
		chunk->Update(camera, deltaTime, renderDebug, renderFoliage);

	m_waterTime += deltaTime;

	m_firstFrame = false;
}

void Terrain::UpdateWater(Camera* camera, float deltaTime, bool renderDebug)
{
	for (auto& chunk : m_chunksList)
		chunk->UpdateWater(camera, deltaTime, renderDebug);

	m_waterMoveFactor += deltaTime * WATER_MOVE_SPEED;
	if (m_waterMoveFactor >= 1.0f)
		m_waterMoveFactor -= 1.0f;
}

void Terrain::Draw(Camera* camera, Light* light, bool renderFoliage, Texture* refractionTexture, Texture* reflectionTexture, Texture* refractionDepthTexture, Texture* reflectionDepthTexture)
{
	for (auto& chunk : m_chunksList)
		chunk->DrawTerrain(camera, light, m_terrainMaterials, m_terrainBiomesData);

	if (refractionTexture && reflectionTexture && refractionDepthTexture && reflectionDepthTexture)
		for (auto& chunk : m_chunksList)
			chunk->DrawWater(camera, light, refractionTexture, reflectionTexture, refractionDepthTexture, reflectionDepthTexture, m_waterMoveFactor, m_waterMaterial, m_waterTime);
	
	if (renderFoliage)
		for (auto& chunk : m_chunksList)
			chunk->DrawFolliage(camera, light);
}

void Terrain::CreateTerrainObjects()
{
	          m_noise                    = new PerlinNoise();
			  m_hydraulicErosion         = new HydraulicErosion( {102400, 0, 3} );
			  m_gaussianBlur             = new GaussianBlur(2.0f);

	Material* snow2                      = new Material("Assets/snow_02_diff_1k.png",             "Assets/snow_02_nor_gl_1k.png",             "Assets/snow_02_spec_1k.png");
	Material* medievalBlocks             = new Material("Assets/medieval_blocks_02_diff_1k.png",  "Assets/medieval_blocks_02_nor_gl_1k.png",  "Assets/medieval_blocks_02_spec_1k.png");
	Material* brownMudLeaves             = new Material("Assets/brown_mud_leaves_01_diff_1k.png", "Assets/brown_mud_leaves_01_nor_gl_1k.png", "Assets/brown_mud_leaves_01_spec_1k.png");
	Material* forestLeaves               = new Material("Assets/forest_leaves_03_diff_1k.png",    "Assets/forest_leaves_03_nor_gl_1k.png");
	Material* snowFieldAerial            = new Material("Assets/snow_field_aerial_col_1k.png",    "Assets/snow_field_aerial_nor_gl_1k.png");
	Material* snow3                      = new Material("Assets/snow_03_diff_1k.png",             "Assets/snow_03_nor_gl_1k.png",             "Assets/snow_03_spec_1k.png");

	ShaderManager* shaderManager = ShaderManager::GetInstance();

	Biome::FolliageModel rockModel = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/Rock/Rock2.obj", true), shaderManager->GetFolliageShader(), 0.25f, 1.0f)
		}, 0.1f);

	Biome::FolliageModel grassModel = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/grass.obj",         true), shaderManager->GetFolliageShader(),           0.05f, 0.05f),
			Biome::ModelLevelOfDetail(new Model("Assets/Models/GrassBilboard.png", true), shaderManager->GetFolliageBilboardedShader(), 5.0f,   1.0f, true)
		}, 1.0f);

	Biome::FolliageModel reedModel = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/Reeds/pxfuel.com.png", true), shaderManager->GetFolliageBilboardedShader(), 10.0f, 1.0f, true)
		}, 1.0f);

	Biome::FolliageModel tree1Model = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/Tree/Tree01.png", true), shaderManager->GetFolliageBilboardedShader(), 10.0f, 1.0f, true)
		}, 2.0f);

	Biome::FolliageModel tree2Model = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/Tree/Tree02.png", true), shaderManager->GetFolliageBilboardedShader(), 10.0f, 1.0f, true)
		}, 2.0f);

	Biome::FolliageModel tree3Model = Biome::FolliageModel(
		{
			Biome::ModelLevelOfDetail(new Model("Assets/Models/Tree/Tree03.png", true), shaderManager->GetFolliageBilboardedShader(), 10.0f, 1.0f, true)
		}, 1.0f);

	Biome* iceBiome = Biome::CreateBiome();

	iceBiome->AddTerrainLevel(snowFieldAerial, { rockModel, tree3Model }, { rockModel });
	iceBiome->AddTerrainLevel(snow3, { rockModel, tree3Model });
	iceBiome->AddTerrainLevel(snow2, { rockModel, tree3Model });
	iceBiome->AddTerrainLevel(snow2, { rockModel });

	Biome* forestBiome = Biome::CreateBiome();

	forestBiome->AddTerrainLevel(forestLeaves, { rockModel, grassModel, tree1Model, tree2Model }, { rockModel, reedModel });
	forestBiome->AddTerrainLevel(brownMudLeaves, { rockModel, grassModel, tree1Model });
	forestBiome->AddTerrainLevel(medievalBlocks, { rockModel });
	forestBiome->AddTerrainLevel(snow3, { rockModel });

	m_terrainBiomesData = Biome::CreateBiomesTexture();
	m_terrainMaterials  = Biome::GetBiomesMaterials();

	m_waterMaterial = new Material("Assets/Water/WaterColor.jpg", "Assets/Water/WaterNormal.jpg");
}

void Terrain::FreeTerrainObjects()
{
	Biome::Free();

	if (m_waterMaterial)
	{
		delete m_waterMaterial;
		m_waterMaterial = nullptr;
	}

	if (m_gaussianBlur)
	{
		delete m_gaussianBlur;
		m_gaussianBlur = nullptr;
	}

	if (m_hydraulicErosion)
	{
		delete m_hydraulicErosion;
		m_hydraulicErosion = nullptr;
	}

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
				Chunk* chunk = new Chunk(m_noise, m_hydraulicErosion, m_gaussianBlur, targetChunk);
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
