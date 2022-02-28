#include <iostream>

#include "Model.h"

using namespace std;
using namespace glm;
using namespace Assimp;

Model::Model(const string& path) :
	m_directory("")
{
	LoadModel(path);
}

Model::~Model()
{
	for (auto& mesh : m_meshes)
	{
		if (mesh)
		{
			for (auto& material : mesh->GetMaterials())
			{
				if (material)
				{
					delete material;
					material = nullptr;
				}
			}

			delete mesh;
			mesh = nullptr;
		}
	}

	m_meshes.clear();
}

int Model::Draw(Shader* shader, const string& texturesName, const string& normalTexturesName, const string& specularTexturesName, int startingTextureNumber)
{
	int resultTextureNumber = startingTextureNumber;

	for (auto& mesh : m_meshes)
		startingTextureNumber = mesh->Draw(shader, texturesName, normalTexturesName, specularTexturesName, startingTextureNumber);

	return resultTextureNumber;
}

void Model::LoadModel(const string& path)
{
	Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}

	m_directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene);
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<VertexNormalTexture> vertices;
	vector<unsigned int>        indices;
	vector<Material*>           materials;

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		auto& position = mesh->mVertices[i];
		auto& normal   = mesh->mNormals[i];

		VertexNormalTexture vertex;

		vertex.Position = vec3(position.x, position.y, position.z);
		vertex.Normal   = vec3(normal.x,   normal.y,   normal.z);

		if (mesh->mTextureCoords[0])
		{
			auto& texCoord  = mesh->mTextureCoords[0][i];
			vertex.TexCoords = vec2(texCoord.x, texCoord.y);
		}
		else
		{
			vertex.TexCoords = vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		materials.push_back(LoadMaterial(material));
	}

	return new Mesh(vertices, indices, materials);
}

Material* Model::LoadMaterial(aiMaterial* mat)
{
	string diffuseFilename  = "";
	string normalFilename   = "";
	string specularFilename = "";
	
	for (int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
		diffuseFilename = m_directory + "/" + str.C_Str();
	}

	for (int i = 0; i < mat->GetTextureCount(aiTextureType_HEIGHT); i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_HEIGHT, i, &str);
		normalFilename = m_directory + "/" + str.C_Str();
	}

	for (int i = 0; i < mat->GetTextureCount(aiTextureType_SPECULAR); i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_SPECULAR, i, &str);
		specularFilename = m_directory + "/" + str.C_Str();
	}

	return new Material(diffuseFilename, normalFilename, specularFilename);
}
