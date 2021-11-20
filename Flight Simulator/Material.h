#pragma once

#include <string>

#include "Texture.h"

class Material
{
public:

	Material(const std::string, const std::string, const std::string = "");
	~Material();

	Texture* GetTexture()         const;
	Texture* GetNormalTexture()   const;
	Texture* GetSpecularTexture() const;

private:

	Texture* m_texture;
	Texture* m_normalTexture;
	Texture* m_specularTexture;
};