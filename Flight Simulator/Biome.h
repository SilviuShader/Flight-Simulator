#pragma once

#include <unordered_set>

#include "Material.h"
#include "Model.h"

class Biome
{
public:

	struct ModelLevelOfDetail
	{
	public:

		ModelLevelOfDetail(Model* model, float scale = 1.0f, float maxDistance = 1.0f, bool bilboarded = false) :
			Model(model),
			Scale(scale),
			MaxDistance(maxDistance),
			Bilboarded(bilboarded)
		{
		}

		bool operator==(const ModelLevelOfDetail& other) const
		{
			return Model       == other.Model       &&
				   Scale       == other.Scale       &&
			       MaxDistance == other.MaxDistance &&
				   Bilboarded  == other.Bilboarded;
		}

	public:

		Model* Model;
		float  Scale;
		float  MaxDistance;
		bool   Bilboarded;
	};

	struct FolliageModel
	{
	public:

		FolliageModel(std::vector<ModelLevelOfDetail> modelLODs, float chance = 1.0f) :
			ModelLODs(modelLODs),
			Chance(chance)
		{
			std::sort(modelLODs.begin(), modelLODs.end(), [&](const ModelLevelOfDetail& a, const ModelLevelOfDetail& b)
				{
					return a.MaxDistance < b.MaxDistance;
				});
		}

		bool operator==(const FolliageModel& other) const
		{
			bool result = Chance == other.Chance;

			if (!result)
				return result;

			result = result && ModelLODs.size() == other.ModelLODs.size();

			if (!result)
				return result;

			for (int i = 0; i < ModelLODs.size(); i++)
			{
				if (!(ModelLODs[i] == other.ModelLODs[i]))
				{
					result = false;
					break;
				}
			}

			return result;
		}

	public:

		std::vector<ModelLevelOfDetail> ModelLODs;
		float                           Chance;
	};

	struct HashFolliageModel
	{
	public:

		size_t operator() (const FolliageModel& model) const
		{
			size_t res = 17;
			for (auto& lod : model.ModelLODs)
			{
				// TODO: create a hash-combine function.
				res = res * 31 + std::hash<int>()((int)lod.Model);
				res = res * 31 + std::hash<float>()(lod.Scale);
				res = res * 31 + std::hash<float>()(lod.MaxDistance);
				res = res * 31 + std::hash<bool>()(lod.Bilboarded);
			}
			res = res * 31 + std::hash<float>()(model.Chance);

			return res;
		}
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