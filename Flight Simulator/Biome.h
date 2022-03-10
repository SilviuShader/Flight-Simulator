#pragma once

#include <unordered_set>

#include "Material.h"
#include "Model.h"

class Biome
{
public:

	struct FolliageModel
	{
	public:

		FolliageModel(Model* model, float scale = 1.0f, float chance = 1.0f) :
			Model(model),
			Scale(scale),
			Chance(chance)
		{
		}

	public:

		Model* Model;
		float  Scale;
		float  Chance;
	};

	struct FolliageModelsVector
	{
		std::vector<FolliageModel> Models;
		float                      Chance;
	};

	struct TerrainLevel
	{
	public:

		Material*                  Material;
		std::vector<FolliageModel> FolliageModels;
	};


public:

	Biome(const Biome&)          = delete;
	void operator=(const Biome&) = delete;

	~Biome();

	       void                              AddTerrainLevel(Material*, const std::vector<FolliageModel>& = std::vector<FolliageModel>());
				                             
		   std::vector<TerrainLevel>&        GetTerrainLevels();
									         
	static Biome*                            CreateBiome();
									         
	static Texture*                          CreateBiomesTexture();
	static std::vector<Material*>            GetBiomesMaterials();

	static std::vector<FolliageModelsVector> GetBiomeFolliageModels(float, float);

	static void                              Free();

private:

	static std::pair<int, float> StepGradient(int, float);

private:

	Biome();

private:

	       std::vector<TerrainLevel>          m_terrainLevels;

	static std::unordered_map<Material*, int> g_materials;
	static std::unordered_set<Model*>         g_folliageModels;
	static std::vector<Texture*>              g_createdTextures;
	static std::vector<Biome*>                g_biomeInstances;

	static int                                g_levelsPerBiomeCount;
};