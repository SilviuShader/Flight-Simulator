#include "Texture3D.h"

#include "glad/glad.h"

Texture3D::Texture3D(int width, int height, int depth, Texture::Format internalFormat, Texture::Format format, Texture::Filter filter, float* texData)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_3D, m_textureID);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, Texture::GetGLParam(filter));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, Texture::GetGLParam(filter));

	glTexImage3D(GL_TEXTURE_3D, 0, 
		         Texture::GetGLFormat(internalFormat), 
		         width, height, depth, 
		         0, 
		         Texture::GetGLFormat(format), GL_FLOAT, 
		         texData);
}

Texture3D::~Texture3D()
{
	glDeleteTextures(1, &m_textureID);
}

unsigned int Texture3D::GetTextureID() const
{
	return m_textureID;
}
