#pragma once

#include <unordered_set>

#include "Material.h"
#include "Model.h"

class Biome
{
public:

	struct TerrainLevel
	{
		Material*           Material;
		std::vector<Model*> FolliageModels;
	};

public:

	Biome(const Biome&)          = delete;
	void operator=(const Biome&) = delete;

	~Biome();

	       void                       AddTerrainLevel(Material*, const std::vector<Model*>& = std::vector<Model*>());
				                      
		   std::vector<TerrainLevel>& GetTerrainLevels();

	static Biome*                     CreateBiome();

	static Texture*                   CreateBiomesTexture();
	static std::vector<Material*>     GetBiomesMaterials();

	static std::vector<Model*>        GetBiomeModels(float, float);

	static void                       Free();

private:

	Biome();

private:

	       std::vector<TerrainLevel>          m_terrainLevels;

	static std::unordered_map<Material*, int> g_materials;
	static std::unordered_set<Model*>         g_folliageModels;
	static std::vector<Texture*>              g_createdTextures;
	static std::vector<Biome*>                g_biomeInstances;
};