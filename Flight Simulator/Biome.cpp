#include "Biome.h"
#include "MathHelper.h"
#include "Terrain.h"

using namespace std;

unordered_map<Material*, int> Biome::g_materials       = unordered_map<Material*, int>();
unordered_set<Model*>         Biome::g_folliageModels  = unordered_set<Model*>();
vector<Texture*>              Biome::g_createdTextures = vector<Texture*>();
vector<Biome*>                Biome::g_biomeInstances  = vector<Biome*>();

int                           Biome::g_levelsPerBiome  = 1;

Biome::~Biome()
{
}

void Biome::AddTerrainLevel(Material* material, const vector<FolliageModel>& models, const vector<FolliageModel>& waterFolliageModels)
{
	int materialsCount = g_materials.size();

	if (g_materials.find(material) == g_materials.end())
		g_materials[material] = materialsCount;

	for (auto& model : models)
	{
		auto& modelLODs = model.ModelLODs;
		for (auto& actualModel : modelLODs)
		{
			auto& actualModelPtr = actualModel.Model;
			if (g_folliageModels.find(actualModelPtr) == g_folliageModels.end())
				g_folliageModels.insert(actualModelPtr);
		}
	}

	for (auto& model : waterFolliageModels)
	{
		auto& modelLODs = model.ModelLODs;
		for (auto& actualModel : modelLODs)
		{
			auto& actualModelPtr = actualModel.Model;
			if (g_folliageModels.find(actualModelPtr) == g_folliageModels.end())
				g_folliageModels.insert(actualModelPtr);
		}
	}

	m_terrainLevels.push_back(TerrainLevel{ material, models, waterFolliageModels });
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
	int biomesCount         = g_biomeInstances.size();
	    g_levelsPerBiome    = 1;

	for (auto& biomeInstance : g_biomeInstances)
		g_levelsPerBiome = MathHelper::LCM(g_levelsPerBiome,
			                               biomeInstance->m_terrainLevels.size());

	int    biomesDataSize = biomesCount * g_levelsPerBiome;
	float* biomesData     = new float[biomesDataSize];
						  
	int    maxMaterialId  = g_materials.size() - 1;

	for (int biomeIndex = 0; biomeIndex < g_biomeInstances.size(); biomeIndex++)
	{
		auto& biome = g_biomeInstances[biomeIndex];

		for (int levelIndex = 0; levelIndex < biome->m_terrainLevels.size(); levelIndex++)
		{
			int copiesCount = g_levelsPerBiome / biome->m_terrainLevels.size();

			for (int copy = 0; copy < copiesCount; copy++)
			{
				int textureIndex = (levelIndex * copiesCount + copy) * g_biomeInstances.size() + biomeIndex;
				Material* material = biome->m_terrainLevels[levelIndex].Material;
				biomesData[textureIndex] = (float)g_materials[material] / (float)maxMaterialId;
			}
		}
	}

	Texture* resultTexture = new Texture(biomesCount,
		                                 g_levelsPerBiome,
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

vector<Biome::FolliageModelsVector> Biome::GetBiomeFolliageModels(float height, float biome)
{
	auto biomeData                   = StepGradient(g_biomeInstances.size(), biome);
	auto heightData                  = StepGradient(g_levelsPerBiome,        height);
								    
	int  currentBiome                = biomeData.first;
	int  nextBiome                   = min(biomeData.first + 1, (int)g_biomeInstances.size() - 1);
		 						    
	int  currentAltitude             = heightData.first;
	int  nextAltitude                = min(currentAltitude + 1, g_levelsPerBiome - 1);
	int  nextAltitudeInNextBiome     = min(currentAltitude + 1, g_levelsPerBiome - 1);
		 
	int  divCurrentAltitude          = (g_levelsPerBiome / g_biomeInstances[currentBiome]->m_terrainLevels.size());
	int  divCurrentAltitudeNextBiome = (g_levelsPerBiome / g_biomeInstances[nextBiome   ]->m_terrainLevels.size());

	Biome::TerrainLevel& currentLevel           = g_biomeInstances[currentBiome]->m_terrainLevels[currentAltitude / divCurrentAltitude];
	Biome::TerrainLevel& nextBiomeLevel         = g_biomeInstances[nextBiome]->m_terrainLevels[currentAltitude / divCurrentAltitudeNextBiome];
	Biome::TerrainLevel& nextAltitudeLevel      = g_biomeInstances[currentBiome]->m_terrainLevels[nextAltitude / divCurrentAltitude];
	Biome::TerrainLevel& nextBiomeAltitudeLevel = g_biomeInstances[nextBiome]->m_terrainLevels[nextAltitudeInNextBiome / divCurrentAltitudeNextBiome];

	float actualHeght                = height * Terrain::TERRAIN_AMPLITUDE;

	auto currentModels               = (actualHeght > Terrain::WATER_LEVEL) ? currentLevel.FolliageModels           : currentLevel.WaterFolliageModels;
	auto nextBiomeModels             = (actualHeght > Terrain::WATER_LEVEL) ? nextBiomeLevel.FolliageModels         : nextBiomeLevel.WaterFolliageModels;
	auto nextAltitudeModels          = (actualHeght > Terrain::WATER_LEVEL) ? nextAltitudeLevel.FolliageModels      : nextAltitudeLevel.WaterFolliageModels;
	auto nextBiomeAltitudeModels     = (actualHeght > Terrain::WATER_LEVEL) ? nextBiomeAltitudeLevel.FolliageModels : nextBiomeAltitudeLevel.WaterFolliageModels;

	FolliageModelsVector currentVector;
	currentVector.Models = currentModels;
	currentVector.Chance = (1.0f - biomeData.second) * (1.0f - heightData.second);

	FolliageModelsVector nextBiomeVector;
	nextBiomeVector.Models = nextBiomeModels;
	nextBiomeVector.Chance = biomeData.second * (1.0f - heightData.second);

	FolliageModelsVector nextAltitudeVector;
	nextAltitudeVector.Models = nextAltitudeModels;
	nextAltitudeVector.Chance = (1.0f - biomeData.second) * heightData.second;

	FolliageModelsVector nextBiomeAltitudeVector;
	nextBiomeAltitudeVector.Models = nextBiomeAltitudeModels;
	nextBiomeAltitudeVector.Chance = biomeData.second * heightData.second;

	return { currentVector, nextBiomeVector, nextAltitudeVector, nextBiomeAltitudeVector };
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

float Biome::InvesreStepGradient(int totalValues, int altitudeStart)
{
	int intervalsCount = 1 + (totalValues - 1) * 2;
	int actualIndex    = altitudeStart * 2;

	return (float)actualIndex / (float)intervalsCount;
}

Biome::Biome() :
	m_terrainLevels(vector<TerrainLevel>())
{
}