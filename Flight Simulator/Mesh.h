#pragma once

#include <vector>

#include "Texture.h"
#include "VertexTypes.h"
#include "Shader.h"

class Mesh
{
public:

	Mesh(std::vector<VertexNormalTextureBinormalTangent>, std::vector<unsigned int>, std::vector<Material*>, bool);
	~Mesh();

	void                    SetInstances(const std::vector<glm::mat4>&);

	int                     Draw(Shader*, const std::string&, const std::string&, const std::string&, int);
	std::vector<Material*>& GetMaterials();

private:

	void SetupMesh();

private:

	std::vector<VertexNormalTextureBinormalTangent> m_vertices;
	std::vector<unsigned int>                       m_indices;
	std::vector<Material*>                          m_materials;
									                
	unsigned int                                    m_vao;
	unsigned int                                    m_instanceVbo;
	unsigned int                                    m_vbo;
	unsigned int                                    m_ebo;

	bool                                            m_instanced;

	int                                             m_instancesCount;
};