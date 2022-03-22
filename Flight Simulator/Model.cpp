#include <iostream>

#include "Model.h"

using namespace std;
using namespace glm;
using namespace Assimp;

Model::Model(const string& path, bool instanced) :
	m_directory(""),
	m_instanced(instanced)
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

void Model::SetInstances(const vector<mat4>& instances)
{
	if (!m_instanced)
		return;

	for (auto& mesh : m_meshes)
		mesh->SetInstances(instances);
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
	if (GetFileExtension(path) == "png")
	{
		vector<VertexNormalTextureBinormalTangent> vertices
		{
			  // POSITION              NORMAL                  TEX_COORDS        BINORMAL                TANGENT
			{ vec3(-0.5f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f) },
			{ vec3(0.5f,  0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f) },
			{ vec3(0.5f,  1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f) },
			{ vec3(-0.5f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f) }
		};

		vector<unsigned int>                       indices
		{
			0, 1, 2,
			0, 2, 3
		};
		
		vector<Material*>                          materials { new Material(path) };

		m_meshes.push_back(new Mesh(vertices, indices, materials, m_instanced));
	}
	else
	{
		Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}

		m_directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}
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
	vector<VertexNormalTextureBinormalTangent> vertices;
	vector<unsigned int>                       indices;
	vector<Material*>                          materials;

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		auto& position = mesh->mVertices[i];
		auto& normal   = mesh->mNormals[i];
		
		VertexNormalTextureBinormalTangent vertex;

		vertex.Position = vec3(position.x, position.y, position.z);
		vertex.Normal   = vec3(normal.x,   normal.y,   normal.z);
		
		if (mesh->mTextureCoords[0])
		{
			auto& texCoord   = mesh->mTextureCoords[0][i];
			vertex.TexCoords = vec2(texCoord.x, texCoord.y);
		}
		else
		{
			vertex.TexCoords = vec2(0.0f, 0.0f);
		}

		auto& binormal  = mesh->mBitangents[i];
		auto& tangent   = mesh->mTangents[i];

		vertex.Binormal = vec3(binormal.x, binormal.y, binormal.z);
		vertex.Tangent  = vec3(tangent.x,  tangent.y,  tangent.z);

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

	return new Mesh(vertices, indices, materials, m_instanced);
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

string Model::GetFileExtension(const string& filename)
{
	string result = "";

	if (filename.find_last_of(".") != string::npos)
		result = filename.substr(filename.find_last_of(".") + 1);

	transform(result.begin(), result.end(), result.begin(),[](unsigned char c) 
	{
			return std::tolower(c); 
	});

	return result;
}
