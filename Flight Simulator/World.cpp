#include <queue>
#include <unordered_set>
#include <glm/glm.hpp>

#include "World.h"

using namespace std;
using namespace glm;

World::World(int windowWidth, int windowHeight) :
	m_moass(nullptr)
{
	m_light = new Light();
	m_light->SetAmbientColor(vec4(0.75f, 0.75f, 0.75f, 1.0f));
	m_light->SetDiffuseColor(vec4(0.8f, 0.8f, 0.9f, 1.0f));

	m_camera = new Camera(radians(45.0f), (float)windowWidth, (float)windowHeight, 0.1f, 1000.0f);
	m_skybox = new Skybox();

	CreateTerrainObjects();
}

World::~World()
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

void World::ProcessMouseInput(float diffX, float diffY)
{
	m_camera->ProcessMouseInput(diffX, diffY);
}

void World::ProcessKeyboardInput(GLFWwindow* window)
{
	m_camera->ProcessKeybaordInput(window);
}

void World::Update(float deltaTime)
{
	m_camera->Update(deltaTime);

	UpdateChunks();
}

void World::Draw()
{
	m_skybox->Draw(m_camera);

	for (auto& keyVal : m_chunks)
		keyVal.second->Draw(m_light, m_camera, m_terrainMaterials, m_terrainBiomesData);
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

	m_terrainMaterials = vector<Material*>();
	m_terrainMaterials.push_back(new Material("Assets/snow_02_diff_1k.png", "Assets/snow_02_nor_gl_1k.png", "Assets/snow_02_spec_1k.png"));
	m_terrainMaterials.push_back(new Material("Assets/medieval_blocks_02_diff_1k.png", "Assets/medieval_blocks_02_nor_gl_1k.png", "Assets/medieval_blocks_02_spec_1k.png"));
	m_terrainMaterials.push_back(new Material("Assets/brown_mud_leaves_01_diff_1k.png", "Assets/brown_mud_leaves_01_nor_gl_1k.png", "Assets/brown_mud_leaves_01_spec_1k.png"));
	m_terrainMaterials.push_back(new Material("Assets/forest_leaves_03_diff_1k.png", "Assets/forest_leaves_03_nor_gl_1k.png"));
	m_terrainMaterials.push_back(new Material("Assets/snow_field_aerial_col_1k.png", "Assets/snow_field_aerial_nor_gl_1k.png"));
	m_terrainMaterials.push_back(new Material("Assets/snow_03_diff_1k.png", "Assets/snow_03_nor_gl_1k.png", "Assets/snow_03_spec_1k.png"));

	constexpr auto   biomesDataSize = BIOMES_COUNT * MATERIALS_PER_BIOME;
	          float  materialsCount = m_terrainMaterials.size() - 1;
	          float* biomesData     = new float[biomesDataSize];

	biomesData[0] = 2.0f / materialsCount;
	biomesData[1] = 3.0f / materialsCount;

	biomesData[2] = 1.0f / materialsCount;
	biomesData[3] = 5.0f / materialsCount;

	biomesData[4] = 5.0f / materialsCount;
	biomesData[5] = 1.0f / materialsCount;

	biomesData[6] = 0.0f / materialsCount;
	biomesData[7] = 4.0f / materialsCount;

	m_terrainBiomesData = new Texture(biomesData, BIOMES_COUNT, MATERIALS_PER_BIOME);

	if (biomesData)
	{
		delete[] biomesData;
		biomesData = nullptr;
	}
}

void World::FreeTerrainObjects()
{
	if (m_terrainBiomesData)
	{
		delete m_terrainBiomesData;
		m_terrainBiomesData = nullptr;
	}

	for (auto& material : m_terrainMaterials)
	{
		if (material)
		{
			delete material;
			material = nullptr;
		}
	}

	m_terrainMaterials.clear();

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

void World::UpdateChunks()
{
	vec3 cameraPos = m_camera->GetPosition();
	vec3 cameraChunkOrigin = cameraPos - vec3((Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS) / 2.0f, 0.0f, (Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS) / 2.0f);

	pair<int, int> currentID = make_pair(int(cameraChunkOrigin.x / (Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)),
		int(cameraChunkOrigin.z / (Chunk::CHUNK_WIDTH - Chunk::CHUNK_CLOSE_BIAS)));

	queue<pair<int, int>> exploreChunksQueue;
	unordered_set<pair<int, int>, HashPair> targetChunks;
	exploreChunksQueue.push(currentID);
	targetChunks.insert(currentID);

	while (targetChunks.size() < MAX_CHUNKS)
	{
		pair<int, int> currentChunk = exploreChunksQueue.front();
		exploreChunksQueue.pop();

		int dx[] = { -1, 0, 1, 0 };
		int dy[] = { 0, -1, 0, 1 };
		int dirCount = sizeof(dx) / sizeof(int);
		for (int i = 0; i < dirCount; i++)
		{
			if (targetChunks.size() >= MAX_CHUNKS)
				break;

			pair<int, int> neighbour = make_pair(currentChunk.first + dx[i], currentChunk.second + dy[i]);
			if (targetChunks.find(neighbour) == targetChunks.end())
			{
				targetChunks.insert(neighbour);
				exploreChunksQueue.push(neighbour);
			}
		}
	}

	vector<pair<int, int>> toErase;

	for (auto& keyVal : m_chunks)
	{
		if (targetChunks.find(keyVal.first) == targetChunks.end())
		{
			delete keyVal.second;
			keyVal.second = nullptr;

			toErase.push_back(keyVal.first);
		}
	}

	for (auto& id : toErase)
		m_chunks.erase(id);

	for (auto& targetChunk : targetChunks)
	{
		if (m_chunks.find(targetChunk) == m_chunks.end())
		{
			Chunk* chunk = new Chunk(m_noise, m_terrainShader, targetChunk);
			m_chunks[targetChunk] = chunk;
		}
	}
}
