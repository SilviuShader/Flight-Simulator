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

	if (m_colorShader)
	{
		delete m_colorShader;
		m_colorShader = nullptr;
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

Shader* ShaderManager::GetTerrainShader() const
{
	return m_terrainShader;
}

Shader* ShaderManager::GetFolliageShader() const
{
	return m_folliageShader;
}

Shader* ShaderManager::GetFolliageBilboardedShader() const
{
	return m_folliageBilboardedShader;
}

Shader* ShaderManager::GetSkyboxShader() const
{
	return m_skyboxShader;
}

Shader* ShaderManager::GetColorShader() const
{
	return m_colorShader;
}

Shader* ShaderManager::GetMinShader() const
{
	return m_minShader;
}

Shader* ShaderManager::GetMaxShader() const
{
	return m_maxShader;
}

Shader* ShaderManager::GetAverageShader() const
{
	return m_averageShader;
}

ShaderManager::ShaderManager()
{
	m_terrainShader            = new Shader("Shaders/Terrain.vert",          "Shaders/Terrain.frag",
		                                    "Shaders/Terrain.tesc",          "Shaders/Terrain.tese");

	m_folliageShader           = new Shader("Shaders/Folliage.vert",         "Shaders/Folliage.frag");
	m_folliageBilboardedShader = new Shader("Shaders/FolliageBilboard.vert", "Shaders/FolliageBilboard.frag");

	m_skyboxShader             = new Shader("Shaders/Skybox.vert",           "Shaders/Skybox.frag");

	m_colorShader              = new Shader("Shaders/Color.vert",            "Shaders/Color.frag");

	m_minShader                = new Shader("Shaders/MinMipMap.comp");
	m_maxShader                = new Shader("Shaders/MaxMipMap.comp");
	m_averageShader            = new Shader("Shaders/AverageMipMap.comp");
}

