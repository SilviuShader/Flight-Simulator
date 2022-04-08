#pragma once
#include "Texture.h"

class Texture3D
{
public:

	Texture3D(int, int, int,
		     Texture::Format = Texture::Format::RGBA32F,
		     Texture::Format = Texture::Format::RGBA,
		     Texture::Filter = Texture::Filter::Linear,
		     float*          = NULL);
	~Texture3D();

	unsigned int GetTextureID() const;

private:

	unsigned int m_textureID;
};