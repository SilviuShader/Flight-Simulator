#include "ShaderManager.h"

ShaderManager* ShaderManager::g_instance = nullptr;

ShaderManager::~ShaderManager()
{
	if (m_averageShader)
	{
		delete m_averageShader;
		m_averageShader = nullptr;
	}

	if (m_maxShader)
	{
		delete m_maxShader;
		m_maxShader = nullptr;
	}

	if (m_minShader)
	{
		delete m_minShader;
		m_minShader = nullptr;
	}

	if (m_texture3DNormalizeShader)
	{
		delete m_texture3DNormalizeShader;
		m_texture3DNormalizeShader = nullptr;
	}

	if (m_texture2DNormalizeShader)
	{
		delete m_texture2DNormalizeShader;
		m_texture2DNormalizeShader = nullptr;
	}

	if (m_worleyNoiseShader)
	{
		delete m_worleyNoiseShader;
		m_worleyNoiseShader = nullptr;
	}

	if (m_simplexNoiseShader)
	{
		delete m_simplexNoiseShader;
		m_simplexNoiseShader = nullptr;
	}

	if (m_perlinNoiseShader)
	{
		delete m_perlinNoiseShader;
		m_perlinNoiseShader = nullptr;
	}

	if (m_texture3DSliceShader)
	{
		delete m_texture3DSliceShader;
		m_texture3DSliceShader = nullptr;
	}

	if (m_textureShader)
	{
		delete m_textureShader;
		m_textureShader = nullptr;
	}

	if (m_colorShader)
	{
		delete m_colorShader;
		m_colorShader = nullptr;
	}

	if (m_cloudsShader)
	{
		delete m_cloudsShader;
		m_cloudsShader = nullptr;
	}

	if (m_skyboxShader)
	{
		delete m_skyboxShader;
		m_skyboxShader = nullptr;
	}

	if (m_folliageBilboardedShader)
	{
		delete m_folliageBilboardedShader;
		m_folliageBilboardedShader = nullptr;
	}

	if (m_folliageShader)
	{
		delete m_folliageShader;
		m_folliageShader = nullptr;
	}

	if (m_waterShader)
	{
		delete m_waterShader;
		m_waterShader = nullptr;
	}

	if (m_terrainShader)
	{
		delete m_terrainShader;
		m_terrainShader = nullptr;
	}
}

ShaderManager* ShaderManager::GetInstance()
{
	if (!g_instance)
		g_instance = new ShaderManager();

	return g_instance;
}

void ShaderManager::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

Shader* ShaderManager::GetTerrainShader()            const
{										             
	return m_terrainShader;				             
}

Shader* ShaderManager::GetWaterShader()              const
{
	return m_waterShader;
}
										             
Shader* ShaderManager::GetFolliageShader()           const
{
	return m_folliageShader;
}

Shader* ShaderManager::GetFolliageBilboardedShader() const
{
	return m_folliageBilboardedShader;
}

Shader* ShaderManager::GetSkyboxShader()             const
{										             
	return m_skyboxShader;				             
}

Shader* ShaderManager::GetCloudsShader()             const
{
	return m_cloudsShader;
}
										             
Shader* ShaderManager::GetColorShader()              const
{
	return m_colorShader;
}

Shader* ShaderManager::GetTextureShader()            const
{
	return m_textureShader;
}

Shader* ShaderManager::GetTexture3DSliceShader()     const
{
	return m_texture3DSliceShader;
}

Shader* ShaderManager::GetPerlinNoiseShader()        const
{											         
	return m_perlinNoiseShader;				         
}

Shader* ShaderManager::GetSimplexNoiseShader()       const
{
	return m_simplexNoiseShader;
}
											         
Shader* ShaderManager::GetWorleyNoiseShader()        const
{
	return m_worleyNoiseShader;
}

Shader* ShaderManager::GetTexture2DNormalizeShader() const
{
	return m_texture2DNormalizeShader;
}

Shader* ShaderManager::GetTexture3DNormalizeShader() const
{
	return m_texture3DNormalizeShader;
}

Shader* ShaderManager::GetMinShader()                const
{
	return m_minShader;
}

Shader* ShaderManager::GetMaxShader()                const
{
	return m_maxShader;
}

Shader* ShaderManager::GetAverageShader()            const
{
	return m_averageShader;
}

ShaderManager::ShaderManager()
{
	m_terrainShader            = new Shader("Shaders/Terrain.vert",          "Shaders/Terrain.frag",
		                                    "Shaders/Terrain.tesc",          "Shaders/Terrain.tese");

	m_waterShader              = new Shader("Shaders/Water.vert",            "Shaders/Water.frag",
		                                    "Shaders/Water.tesc",            "Shaders/Water.tese");

	m_folliageShader           = new Shader("Shaders/Folliage.vert",         "Shaders/Folliage.frag");
	m_folliageBilboardedShader = new Shader("Shaders/FolliageBilboard.vert", "Shaders/FolliageBilboard.frag");

	m_skyboxShader             = new Shader("Shaders/Skybox.vert",           "Shaders/Skybox.frag");
	m_cloudsShader             = new Shader("Shaders/Clouds.vert",           "Shaders/Clouds.frag");

	m_colorShader              = new Shader("Shaders/Color.vert",            "Shaders/Color.frag");
	m_textureShader            = new Shader("Shaders/Texture.vert",          "Shaders/Texture.frag");
	m_texture3DSliceShader     = new Shader("Shaders/Texture3DSlice.vert",   "Shaders/Texture3DSlice.frag");

	m_perlinNoiseShader        = new Shader("Shaders/PerlinNoise.comp");
	m_simplexNoiseShader       = new Shader("Shaders/SimplexNoise.comp");
	m_worleyNoiseShader        = new Shader("Shaders/WorleyNoise.comp");
	m_texture2DNormalizeShader = new Shader("Shaders/Texture2DNormalize.comp");
	m_texture3DNormalizeShader = new Shader("Shaders/Texture3DNormalize.comp");

	m_minShader                = new Shader("Shaders/MinMipMap.comp");
	m_maxShader                = new Shader("Shaders/MaxMipMap.comp");
	m_averageShader            = new Shader("Shaders/AverageMipMap.comp");
}

