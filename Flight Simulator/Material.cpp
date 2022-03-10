#include "Material.h"

#include "glad/glad.h"

using namespace std;

Material::Material(const string textureFilename, const string normalTextureFilename, const string specularTextureFilename)
{
	if (textureFilename.size())
	{
		m_texture = new Texture(textureFilename);
	}
	else
	{
		float* textureData = new float[16];
		for (int i = 0; i < 16;)
		{
			if (i == 0 || i == 8)
			{
				textureData[i++] = 1.0f;
				textureData[i++] = 0.0f;
				textureData[i++] = 1.0f;
				textureData[i++] = 1.0f;
			}
			else
			{
				textureData[i++] = 0.0f;
				textureData[i++] = 0.0f;
				textureData[i++] = 0.0f;
				textureData[i++] = 1.0f;
			}
		}

		m_texture = new Texture(4, 
			                    4,
			                    Texture::Format::RGBA,
			                    Texture::Format::RGBA,
			                    Texture::Filter::Point,
			                    textureData);

		if (textureData)
		{
			delete[] textureData;
			textureData = nullptr;
		}
	}

	int textureSize = m_texture->GetWidth() * m_texture->GetHeight();

	if (normalTextureFilename.size())
	{
		m_normalTexture = new Texture(normalTextureFilename);
	}
	else
	{
		int normalWidth = 4;
		int normalHeight = 4;
		float* normalValues = new float[normalWidth * normalHeight * 4];

		for (int i = 0; i < normalWidth * normalHeight * 4;)
		{
			normalValues[i++] = 0.5f;
			normalValues[i++] = 0.5f;
			normalValues[i++] = 1.0f;
			normalValues[i++] = 1.0f;
		}

		m_normalTexture = new Texture(normalWidth,
			                          normalHeight,
			                          Texture::Format::RGBA,
			                          Texture::Format::RGBA,
			                          Texture::Filter::Linear,
			                          normalValues);

		if (normalValues)
		{
			delete[] normalValues;
			normalValues = nullptr;
		}
	}

	if (specularTextureFilename.size())
	{
		m_specularTexture = new Texture(specularTextureFilename);
	}
	else
	{
		int specularWidth = 4;
		int specularHeight = 4;
		float* specularValues = new float[specularWidth * specularHeight];
		for (int i = 0; i < specularWidth * specularHeight; i++)
			specularValues[i] = 0.0f;

		m_specularTexture = new Texture(specularWidth, 
			                            specularHeight, 
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
