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
		   Shader*        GetWaterShader()              const;
											            
		   Shader*        GetFolliageShader()           const;
		   Shader*        GetFolliageBilboardedShader() const;

		   Shader*        GetSkyboxShader()             const;
		   Shader*        GetCloudsShader()             const;

		   Shader*        GetColorShader()              const;
		   Shader*        GetTextureShader()            const;
		   Shader*        GetTexture3DSliceShader()     const;

		   Shader*        GetPerlinNoiseShader()        const;
		   Shader*        GetSimplexNoiseShader()       const;
		   Shader*        GetWorleyNoiseShader()        const;
		   Shader*        GetTexture2DNormalizeShader() const;
		   Shader*        GetTexture3DNormalizeShader() const;

		   Shader*        GetHydraulicErosionShader()   const;

		   Shader*        GetMinShader()                const;
		   Shader*        GetMaxShader()                const;
		   Shader*        GetAverageShader()            const;
		   Shader*        GetGaussianBlurShader()       const;

private:

	ShaderManager();

private:

	       Shader*        m_terrainShader;
		   Shader*        m_waterShader;

		   Shader*        m_folliageShader;
		   Shader*        m_folliageBilboardedShader;

		   Shader*        m_skyboxShader;
		   Shader*        m_cloudsShader;

		   Shader*        m_colorShader;
		   Shader*        m_textureShader;
		   Shader*        m_texture3DSliceShader;

		   Shader*        m_perlinNoiseShader;
		   Shader*        m_simplexNoiseShader;
		   Shader*        m_worleyNoiseShader;
		   Shader*        m_texture2DNormalizeShader;
		   Shader*        m_texture3DNormalizeShader;

		   Shader*        m_hydraulicErosionShader;
	       		          
	       Shader*        m_minShader;
	       Shader*        m_maxShader;
	       Shader*        m_averageShader;
		   Shader*        m_gaussianBlurShader;

	static ShaderManager* g_instance;
};