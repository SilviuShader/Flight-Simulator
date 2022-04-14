#pragma once

#include <glm/glm.hpp>

#include "Camera.h"
#include "Shader.h"

class DebugHelper
{
public:

	DebugHelper(const DebugHelper&)    = delete;
	void operator=(const DebugHelper&) = delete;

	~DebugHelper();

	static DebugHelper* GetInstance();
	static void         FreeInstance();
				        
		   void         ResetInstances();
		   void         AddRectangleInstance(const glm::vec3&, const glm::vec3&);
		   void         DrawRectangles(Camera*);

		   void         DrawFullscreenTexture(Texture*);
		   void         DrawTexture3DSlice(Texture3D*, float, float);

		   void         FullScreenQuadDrawCall();

private:

	DebugHelper();

	void CreateCubeBuffers();
	void FreeCubeBuffers();

	void CreateQuadBuffers();
	void FreeQuadBuffers();

private:

	       unsigned int           m_cubeVBO;
		   unsigned int           m_cubeInstanceVbo;
	       unsigned int           m_cubeEBO;
	       unsigned int           m_cubeVAO;

		   std::vector<glm::mat4> m_rectangleInstances;

		   unsigned int           m_quadVBO;
		   unsigned int           m_quadEBO;
		   unsigned int           m_quadVAO;
		   
	static DebugHelper*           g_instance;
};