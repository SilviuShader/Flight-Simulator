#pragma once
#include <vector>
#include <unordered_map>
#include "Chunk.h"

class Terrain
{
private:

	const int   MAX_CHUNKS                    = 54;
	const float TIME_TO_UPDATE_CURRENT_CHUNKS = 0.5;

public:

	Terrain();
	~Terrain();

	void Udpate(Camera*, float, bool);
	void Draw(Light*);

private:

	void CreateTerrainObjects();
	void FreeTerrainObjects();

	void UpdateChunksVisibility(Camera*, float, int);
	void UpdateCurrentChunks(Camera*, float);

private:


	std::unordered_map<Vec2Int, Chunk*, HashPair> m_chunks;
	std::vector<Chunk*>                           m_chunksList;

	PerlinNoise*                                  m_noise;
	Shader*                                       m_terrainShader;
	Shader*                                       m_folliageShader;

	std::vector<Material*>                        m_terrainMaterials;
	Texture*                                      m_terrainBiomesData;

	Shader*                                       m_minShader;
	Shader*                                       m_maxShader;
	Shader*                                       m_averageShader;

	float                                         m_accumulatedCurrentChunksTime;

	bool                                          m_firstFrame;
};