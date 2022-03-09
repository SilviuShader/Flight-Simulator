#include "Biome.h"

using namespace std;

unordered_map<Material*, int> Biome::g_materials       = unordered_map<Material*, int>();
unordered_set<Model*>         Biome::g_folliageModels  = unordered_set<Model*>();
vector<Texture*>              Biome::g_createdTextures = vector<Texture*>();
vector<Biome*>                Biome::g_biomeInstances  = vector<Biome*>();

Biome::~Biome()
{
}

void Biome::AddTerrainLevel(Material* material, const std::vector<Model*>& models)
{
	int materialsCount = g_materials.size();

	if (g_materials.find(material) == g_materials.end())
		g_materials[material] = materialsCount;

	for (auto& model : models)
		if (g_folliageModels.find(model) == g_folliageModels.end())
			g_folliageModels.insert(model);

	m_terrainLevels.push_back(TerrainLevel{ material, models });
}

vector<Biome::TerrainLevel>& Biome::GetTerrainLevels()
{
	return m_terrainLevels;
}

Biome* Biome::CreateBiome()
{
	Biome* result = new Biome();
	g_biomeInstances.push_back(result);
	return result;
}

Texture* Biome::CreateBiomesTexture()
{
	int biomesCount       = g_biomeInstances.size();
	int materialsPerBiome = 0;
	
	for (auto& biome : g_biomeInstances)
		materialsPerBiome = max(materialsPerBiome, (int)biome->m_terrainLevels.size());

	int    biomesDataSize = biomesCount * materialsPerBiome;
	float* biomesData     = new float[biomesDataSize];

	int maxMaterialId     = g_materials.size() - 1;

	for (int biomeIndex = 0; biomeIndex < g_biomeInstances.size(); biomeIndex++)
	{
		auto& biome = g_biomeInstances[biomeIndex];

		for (int levelIndex = 0; levelIndex < biome->m_terrainLevels.size(); levelIndex++)
		{
			int textureIndex = levelIndex * g_biomeInstances.size() + biomeIndex;
			Material* material = biome->m_terrainLevels[levelIndex].Material;
			biomesData[textureIndex] = (float)g_materials[material] / (float)maxMaterialId;
		}
	}

	Texture* resultTexture = new Texture(biomesCount,
		                                 materialsPerBiome,
		                                 Texture::Format::RED,
		                                 Texture::Format::RED,
		                                 Texture::Filter::Point,
		                                 biomesData);

	g_createdTextures.push_back(resultTexture);

	return resultTexture;
}

vector<Material*> Biome::GetBiomesMaterials()
{
	vector<Material*> result(g_materials.size());

	for (auto& material : g_materials)
		result[material.second] = material.first;

	return result;
}

vector<Model*> Biome::GetBiomeModels(float biome, float amplitude)
{
	// TODO: Properly implement this method.
	for (auto& biome : g_biomeInstances)
		for (auto& level : biome->m_terrainLevels)
			if (level.FolliageModels.size())
				return level.FolliageModels;
	

	return vector<Model*>();
}

void Biome::Free()
{
	for (auto& texture : g_createdTextures)
	{
		if (texture)
		{
			delete texture;
			texture = nullptr;
		}
	}

	g_createdTextures.clear();

	for (auto& biome : g_biomeInstances)
	{
		if (biome)
		{
			delete biome;
			biome = nullptr;
		}
	}

	g_biomeInstances.clear();

	for (auto& material : g_materials)
		if (material.first)
			delete material.first;

	g_materials.clear();

	for (auto& model : g_folliageModels)
		if (model)
			delete model;

	g_folliageModels.clear();	
}

Biome::Biome() :
	m_terrainLevels(vector<TerrainLevel>())
{
}