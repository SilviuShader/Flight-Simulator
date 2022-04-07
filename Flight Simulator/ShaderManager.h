#pragma once

#include "Shader.h"

class ShaderManager
{
public:

	ShaderManager(const ShaderManager&)  = delete;
	void operator=(const ShaderManager&) = delete;

	~ShaderManager();

	static ShaderManager* GetInstance();
	static void           FreeInstance();

	       Shader*        GetTerrainShader()            const;
											            
		   Shader*        GetFolliageShader()           const;
		   Shader*        GetFolliageBilboardedShader() const;

		   Shader*        GetSkyboxShader()             const;

		   Shader*        GetColorShader()              const;

		   Shader*        GetMinShader()                const;
		   Shader*        GetMaxShader()                const;
		   Shader*        GetAverageShader()            const;

private:

	ShaderManager();

private:

	       Shader*        m_terrainShader;

		   Shader*        m_folliageShader;
		   Shader*        m_folliageBilboardedShader;

		   Shader*        m_skyboxShader;

		   Shader*        m_colorShader;
	       		          
	       Shader*        m_minShader;
	       Shader*        m_maxShader;
	       Shader*        m_averageShader;

	static ShaderManager* g_instance;
};