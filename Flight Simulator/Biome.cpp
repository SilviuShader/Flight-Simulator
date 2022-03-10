#include "Biome.h"

using namespace std;

unordered_map<Material*, int> Biome::g_materials              = unordered_map<Material*, int>();
unordered_set<Model*>         Biome::g_folliageModels         = unordered_set<Model*>();
vector<Texture*>              Biome::g_createdTextures        = vector<Texture*>();
vector<Biome*>                Biome::g_biomeInstances         = vector<Biome*>();

int                           Biome::g_levelsPerBiomeCount = 0;

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

	g_levelsPerBiomeCount = max(g_levelsPerBiomeCount, (int)m_terrainLevels.size());
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

	int    biomesDataSize = biomesCount * g_levelsPerBiomeCount;
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
		                                 g_levelsPerBiomeCount,
		                                 Texture::Format::RED,
		                                 Texture::Format::RED,
		                                 Texture::Filter::Point,
		                                 biomesData);

	g_createdTextures.push_back(resultTexture);

	if (biomesData)
	{
		delete[] biomesData;
		biomesData = nullptr;
	}

	return resultTexture;
}

vector<Material*> Biome::GetBiomesMaterials()
{
	vector<Material*> result(g_materials.size());

	for (auto& material : g_materials)
		result[material.second] = material.first;

	return result;
}

vector<Model*> Biome::GetBiomeModels(float height, float biome)
{
	auto biomeData  = StepGradient(g_biomeInstances.size(), biome);
	auto heightData = StepGradient(g_levelsPerBiomeCount,   height);

	// TODO: Code all the interpolations here.

	return g_biomeInstances[biomeData.first]->m_terrainLevels[heightData.first].FolliageModels;
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

	g_levelsPerBiomeCount = 0;
}

pair<int, float> Biome::StepGradient(int totalValues, float t)
{
	int index;
	float percentage;

	int intervalsCount = 1 + (totalValues - 1) * 2;
	float floatVal = t * intervalsCount;

	int intVal = int(floor(floatVal));
	if (intVal >= intervalsCount)
		intVal = intervalsCount - 1;

	if (intVal % 2 == 0)
	{
		index = (intVal / 2);
		percentage = 0.0;
	}
	else
	{
		index = (intVal - 1) / 2;
		percentage = floatVal - intVal;
	}

	return make_pair(index, percentage);
}

Biome::Biome() :
	m_terrainLevels(vector<TerrainLevel>())
{
}