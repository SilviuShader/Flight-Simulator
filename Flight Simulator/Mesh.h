#pragma once

#include <vector>

#include "Texture.h"
#include "VertexTypes.h"
#include "Shader.h"

class Mesh
{
public:

	Mesh(std::vector<VertexNormalTexture>, std::vector<unsigned int>, std::vector<Material*>);
	~Mesh();

	int Draw(Shader*, const std::string&, const std::string&, const std::string&, int);

private:

	void SetupMesh();

private:

	std::vector<VertexNormalTexture> m_vertices;
	std::vector<unsigned int>        m_indices;
	std::vector<Material*>           m_materials;

	unsigned int                     m_vao;
	unsigned int                     m_vbo;
	unsigned int                     m_ebo;
};