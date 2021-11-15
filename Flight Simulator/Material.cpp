#include "Material.h"

using namespace std;

Material::Material(const string textureFilename, const string normalTextureFilename, const string specularTextureFilename)
{
	m_texture         = new Texture(textureFilename);
	m_normalTexture   = new Texture(normalTextureFilename);
	m_specularTexture = new Texture(specularTextureFilename);
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
