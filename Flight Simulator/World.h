#pragma once

#include <iostream>
#include <bitset>
#include <unordered_map>

#include "Skybox.h"
#include "PerlinNoise.h"
#include "Chunk.h"
#include "Camera.h"
#include "Utils.h"
#include "Model.h"

class World
{
private:

	static const int   BIOMES_COUNT                  = 2;
	static const int   MATERIALS_PER_BIOME           = 4;
					   					             
	       const int   MAX_CHUNKS                    = 64;

		   const float TIME_TO_UPDATE_CURRENT_CHUNKS = 0.5;

public:

	World(int, int);
	~World();

	void    UpdateWindowSize(int, int);

	void    Update(float);
	void    Draw();

	Camera* GetCamera() const;

private:

	void CreateTerrainObjects();
	void FreeTerrainObjects();

	void UpdateChunksVisibility(float, int);
	void UpdateCurrentChunks(float);

private:

	Light*                                        m_light;
	Camera*                                       m_camera;

	Skybox*                                       m_skybox;
	std::unordered_map<Vec2Int, Chunk*, HashPair> m_chunks;

	// Terrain objects
	PerlinNoise*                                  m_noise;
	Shader*                                       m_terrainShader;
	std::vector<Material*>                        m_terrainMaterials;
	Texture*                                      m_terrainBiomesData;

	Model*                                        m_grassModel;
	Shader*                                       m_modelShader;

	float                                         m_accumulatedCurrentChunksTime;

	bool                                          m_firstFrame;
	bool                                          m_renderDebug;
};