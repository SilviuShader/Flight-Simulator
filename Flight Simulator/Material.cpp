#include "Material.h"

#include "glad/glad.h"

using namespace std;

Material::Material(const string textureFilename, const string normalTextureFilename, const string specularTextureFilename)
{
	m_texture       = new Texture(textureFilename);
	m_normalTexture = new Texture(normalTextureFilename);

	if (specularTextureFilename.size())
	{
		m_specularTexture = new Texture(specularTextureFilename);
	}
	else
	{
		int textureSize = m_texture->GetWidth() * m_texture->GetHeight();
		float* specularValues = new float[textureSize];
		for (int i = 0; i < textureSize; i++)
			specularValues[i] = 0.0f;

		m_specularTexture = new Texture(m_texture->GetWidth(), 
			                            m_texture->GetHeight(), 
			                            Texture::Format::RED, 
			                            Texture::Format::RED, 
			                            Texture::Filter::Linear, 
			                            specularValues);

		if (specularValues)
		{
			delete[] specularValues;
			specularValues = nullptr;
		}
	}
}

Material::~Material()
{
	if (m_specularTexture)
	{
		delete m_specularTexture;
		m_specularTexture = nullptr;
	}

	if (m_normalTexture)
	{
		delete m_normalTexture;
		m_normalTexture = nullptr;
	}

	if (m_texture)
	{
		delete m_texture;
		m_texture = nullptr;
	}
}

Texture* Material::GetTexture() const
{
	return m_texture;
}

Texture* Material::GetNormalTexture() const
{
	return m_normalTexture;
}

Texture* Material::GetSpecularTexture() const
{
	return m_specularTexture;
}
