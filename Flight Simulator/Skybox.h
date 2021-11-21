#pragma once

#include "Camera.h"
#include "Shader.h"

class Skybox
{
public:

	Skybox();
	~Skybox();

	void Draw(Camera*);

private:

	void CreateCubeBuffers();
	void FreeCubeBuffers();

private:

	unsigned int m_vbo;
	unsigned int m_ebo;
	unsigned int m_vao;

	Shader*      m_skyboxShader;
};