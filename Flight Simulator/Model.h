#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

// TODO: ADD TANGENT AND BINORMAL INFO
class Model
{
public:

	Model(const std::string&);
	~Model();

	int Draw(Shader*, const std::string&, const std::string&, const std::string&, int);

private:

	void      LoadModel(const std::string&);
	void      ProcessNode(aiNode*, const aiScene*);
	Mesh*     ProcessMesh(aiMesh*, const aiScene*);

	Material* LoadMaterial(aiMaterial*);

private:

	std::vector<Mesh*> m_meshes;
	std::string        m_directory;
};