#pragma once

#include <glm/glm.hpp>

#include "Camera.h"
#include "Shader.h"

class Shapes
{
private:

	struct Vertex
	{
	public:

		Vertex();
		Vertex(glm::vec3, glm::vec3);

	public:

		glm::vec3 Position;
		glm::vec3 Color;
	};

public:

	Shapes(const Shapes&)         = delete;
	void operator=(const Shapes&) = delete;

	~Shapes();

	static Shapes* GetInstance();
	static void    FreeInstance();

		   void    DrawRectangle(const glm::vec3&, const glm::vec3&, Camera*);

private:

	Shapes();

	void CreateCubeBuffers();
	void FreeCubeBuffers();

private:

	unsigned int m_vbo;
	unsigned int m_ebo;
	unsigned int m_vao;
	
	Shader*      m_colorShader;

private:

	static Shapes* g_instance;
};