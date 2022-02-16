#include "TextureLoadHelper.h"
#include "stb_image.h"

using namespace std;

TextureLoadHelper* TextureLoadHelper::g_instance = nullptr;

void TextureLoadHelper::SetFlipVerticallyOnLoad(bool flipVerticallyOnLoad)
{
	stbi_set_flip_vertically_on_load(flipVerticallyOnLoad);
	m_flipVerticallyOnLoad = flipVerticallyOnLoad;
}

TextureLoadHelper::ImageData TextureLoadHelper::LoadImage(const string& filename)
{
	ImageData result;
	result.Data = stbi_load(filename.c_str(), &result.Width, &result.Height, &result.ChannelsCount, 0);
	return result;
}

void TextureLoadHelper::FreeImage(ImageData& imageData)
{
	if (imageData.Data)
	{
		stbi_image_free(imageData.Data);
		imageData.Data = nullptr;
	}

	imageData.Width         = 0;
	imageData.Height        = 0;
	imageData.ChannelsCount = 0;
}

bool TextureLoadHelper::GetFlipVerticallyOnLoad() const
{
	return m_flipVerticallyOnLoad;
}

TextureLoadHelper* TextureLoadHelper::GetInstance()
{
	if (!g_instance)
		g_instance = new TextureLoadHelper();

	return g_instance;
}

void TextureLoadHelper::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

TextureLoadHelper::TextureLoadHelper() :
	m_flipVerticallyOnLoad(false)
{
}
