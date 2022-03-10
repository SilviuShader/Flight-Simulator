#include <queue>
#include <unordered_set>
#include <glm/glm.hpp>

#include "World.h"
#include "InputWrapper.h"
#include "Shapes.h"
#include <glm/ext/matrix_transform.hpp>
#include "Biome.h"

using namespace std;
using namespace glm;

World::World(int windowWidth, int windowHeight) :
	m_renderDebug(false),
	m_accumulatedCurrentChunksTime(0.0f),
	m_firstFrame(true)
{
	m_light = new Light();
	m_light->SetAmbientColor(vec4(0.2f, 0.2f, 0.2f, 1.0f));
	m_light->SetDiffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_light->SetSpecularPower(4.0f);

	m_camera = new Camera(radians(45.0f), (float)windowWidth, (float)windowHeight, 0.1f, 1000.0f);
	m_skybox = new Skybox();

	CreateTerrainObjects();

	m_folliageShader = new Shader("Shaders/Folliage.vert", "Shaders/Folliage.frag");
}

World::~World()
{
	if (m_folliageShader)
	{
		delete m_folliageShader;
		m_folliageShader = nullptr;
	}

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

	if (m_skybox)
	{
		delete m_skybox;
		m_skybox = nullptr;
	}

	if (m_camera)
	{
		delete m_camera;
		m_camera = nullptr;
	}

	if (m_light)
	{
		delete m_light;
		m_light = nullptr;
	}
}

void World::UpdateWindowSize(int width, int height)
{
	m_camera->UpdateWindowSize(width, height);
}

void World::Update(float deltaTime)
{
	m_camera->Update(deltaTime);

	if (InputWrapper::GetInstance()->GetKeyUp(InputWrapper::Keys::Debug))
		m_renderDebug = !m_renderDebug;

	if (m_renderDebug)
		Shapes::GetInstance()->ResetInstances();

	UpdateCurrentChunks(deltaTime);

	for (auto& keyVal : m_chunks)
		keyVal.second->Update(m_camera, deltaTime, m_renderDebug);

	m_firstFrame = false;
}

void World::Draw()
{
	m_skybox->Draw(m_camera);

	for (auto& keyVal : m_chunks)
		keyVal.second->Draw(m_light, m_terrainMaterials, m_terrainBiomesData);

	if (m_renderDebug)
		Shapes::GetInstance()->DrawRectangles(m_camera);
}

Camera* World::GetCamera() const
{
	return m_camera;
}

void World::CreateTerrainObjects()
{
	m_noise         = new PerlinNoise();
	m_terrainShader = new Shader("Shaders/Terrain.vert", "Shaders/Terrain.frag",
		                         "Shaders/Terrain.tesc", "Shaders/Terrain.tese");

	Material* snow2           = new Material("Assets/snow_02_diff_1k.png",             "Assets/snow_02_nor_gl_1k.png",             "Assets/snow_02_spec_1k.png");
	Material* medievalBlocks  = new Material("Assets/medieval_blocks_02_diff_1k.png",  "Assets/medieval_blocks_02_nor_gl_1k.png",  "Assets/medieval_blocks_02_spec_1k.png");
	Material* brownMudLeaves  = new Material("Assets/brown_mud_leaves_01_diff_1k.png", "Assets/brown_mud_leaves_01_nor_gl_1k.png", "Assets/brown_mud_leaves_01_spec_1k.png");
	Material* forestLeaves    = new Material("Assets/forest_leaves_03_diff_1k.png",    "Assets/forest_leaves_03_nor_gl_1k.png");
	Material* snowFieldAerial = new Material("Assets/snow_field_aerial_col_1k.png",    "Assets/snow_field_aerial_nor_gl_1k.png");
	Material* snow3           = new Material("Assets/snow_03_diff_1k.png",             "Assets/snow_03_nor_gl_1k.png",             "Assets/snow_03_spec_1k.png");


	Biome* iceBiome = Biome::CreateBiome();

	iceBiome->AddTerrainLevel(snowFieldAerial);
	iceBiome->AddTerrainLevel(snow3);
	iceBiome->AddTerrainLevel(snow2);
	iceBiome->AddTerrainLevel(snow2);

	Biome* forestBiome = Biome::CreateBiome();

	forestBiome->AddTerrainLevel(forestLeaves,   { Biome::FolliageModel(new Model("Assets/Models/grass.obj",    true), 0.05f) });
	forestBiome->AddTerrainLevel(brownMudLeaves, { Biome::FolliageModel(new Model("Assets/Models/backpack.obj", true))        });
	forestBiome->AddTerrainLevel(medievalBlocks, { Biome::FolliageModel(new Model("Assets/Models/backpack.obj", true))        });
	forestBiome->AddTerrainLevel(snow3);

	m_terrainBiomesData = Biome::CreateBiomesTexture();
	m_terrainMaterials  = Biome::GetBiomesMaterials();
}

void World::FreeTerrainObjects()
{
	Biome::Free();

	if (m_terrainShader)
	{
		delete m_terrainShader;
		m_terrainShader = nullptr;
	}

	if (m_noise)
	{
		delete m_noise;
		m_noise = nullptr;
	}
}

void World::UpdateChunksVisibility(float deltaTime, int diffSize)
{
	vec3                             cameraPos         = m_camera->GetPosition();
	vec3                             cameraChunkOrigin = cameraPos - vec3((Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS) / 2.0f, 
		                                                                 0.0f, 
		                                                                 (Chunk::CHUNK_WIDTH  - Chunk::CHUNK_CLOSE_BIAS) / 2.0f);

	Vec2Int                          currentID         = make_pair(int(cameraChunkOrigin.x / (Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)),
		                                                           int(cameraChunkOrigin.z / (Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)));

	queue<Vec2Int>                   exploreChunksQueue;
	unordered_set<Vec2Int, HashPair> targetChunksSet;

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
				Chunk* chunk = new Chunk(m_noise, m_terrainShader, targetChunk, m_folliageShader);
				m_chunks[targetChunk] = chunk;

				additions++;
			}
		}
	}
}

void World::UpdateCurrentChunks(float deltaTime)
{
	m_accumulatedCurrentChunksTime += deltaTime;

	if (m_accumulatedCurrentChunksTime < TIME_TO_UPDATE_CURRENT_CHUNKS && !m_firstFrame)
		return;

	m_accumulatedCurrentChunksTime -= m_firstFrame ? 0.0f : TIME_TO_UPDATE_CURRENT_CHUNKS;

	UpdateChunksVisibility(deltaTime, m_firstFrame ? MAX_CHUNKS : 1);
}
