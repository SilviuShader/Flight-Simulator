#pragma once
#include <vector>
#include <unordered_map>
#include "Chunk.h"
#include "HydraulicErosion.h"
#include "GaussianBlur.h"

class Terrain
{
public:

	static const float CHUNK_WIDTH;
	static const float TERRAIN_AMPLITUDE;
	static const float WATER_LEVEL;
	static const float DISTANCE_FOR_DETAILS;
	static const float MAX_TESSELATION;
	static const float GAMMA;

	static const float HEIGHT_FREQUENCY;
	static const float HEIGHT_FUDGE_FACTOR;
	static const float HEIGHT_EXPONENT;
	static const int   HEIGHT_OCTAVES_COUNT;

	static const float BIOME_FREQUENCY;
	static const float BIOME_FUDGE_FACTOR;
	static const float BIOME_EXPONENT;
	static const int   BIOME_OCTAVES_COUNT;

	static const float FOLLIAGE_RANDOMNESS_FREQUENCY;
	static const float FOLLIAGE_RANDOMNESS_FUDGE_FACTOR;
	static const float FOLLIAGE_RANDOMNESS_EXPONENT;
	static const int   FOLLIAGE_RANDOMNESS_OCTAVES_COUNT;

	static const float FOLLIAGE_RANDOMNESS_THRESHOLD;

	static const float FOLLIAGE_SELECTION_RANDOMNESS_FREQUENCY;
	static const float FOLLIAGE_SELECTION_RANDOMNESS_FUDGE_FACTOR;
	static const float FOLLIAGE_SELECTION_RANDOMNESS_EXPONENT;
	static const int   FOLLIAGE_SELECTION_RANDOMNESS_OCTAVES_COUNT;

	static const float WATER_MOVE_SPEED;

private:

	const int   MAX_CHUNKS                    = 54;
	const float TIME_TO_UPDATE_CURRENT_CHUNKS = 0.5;

public:

	Terrain();
	~Terrain();

	void Udpate(Camera*, float, bool, bool);
	void UpdateWater(Camera*, float, bool);
	void Draw(Camera*, Light*, bool, Texture* = nullptr, Texture* = nullptr, Texture* = nullptr, Texture* = nullptr);

private:

	void CreateTerrainObjects();
	void FreeTerrainObjects();

	void UpdateChunksVisibility(Camera*, float, int);
	void UpdateCurrentChunks(Camera*, float);

private:


	std::unordered_map<Vec2Int, Chunk*, HashHelper::HashPair> m_chunks;
	std::vector<Chunk*>                                       m_chunksList;
												              
	PerlinNoise*                                              m_noise;
	HydraulicErosion*                                         m_hydraulicErosion;
	GaussianBlur*                                             m_gaussianBlur;
												              
	std::vector<Material*>                                    m_terrainMaterials;
	Texture*                                                  m_terrainBiomesData;

	Material*                                                 m_waterMaterial;
												              
	float                                                     m_accumulatedCurrentChunksTime;
	float                                                     m_waterTime;
												              
	bool                                                      m_firstFrame;
	float                                                     m_waterMoveFactor;
};