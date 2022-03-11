#include <iostream>

#include "glad/glad.h"
#include "Cubemap.h"
#include "TextureLoadHelper.h"

using namespace std;

Cubemap::Cubemap(const vector<string>& faces)
{
	bool initialFlipConfig = TextureLoadHelper::GetInstance()->GetFlipVerticallyOnLoad();
	TextureLoadHelper::GetInstance()->SetFlipVerticallyOnLoad(false);

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

	for (int i = 0; i < faces.size(); i++)
	{
		TextureLoadHelper::ImageData imageData = TextureLoadHelper::GetInstance()->LoadImage(faces[i]);
		
		if (imageData.Data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, imageData.Width, imageData.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData.Data);
		else
			cout << "Cubemap tex failed to load at path: " << faces[i] << endl;

		TextureLoadHelper::GetInstance()->FreeImage(imageData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	TextureLoadHelper::GetInstance()->SetFlipVerticallyOnLoad(initialFlipConfig);
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &m_textureID);
}

unsigned int Cubemap::GetTextureID() const
{
	return m_textureID;
}

