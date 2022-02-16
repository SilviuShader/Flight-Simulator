#pragma once

#include <string>

class TextureLoadHelper
{
public:

	struct ImageData
	{
		unsigned char* Data;
		int            Width;
		int            Height;
		int            ChannelsCount;
	};

public:

	TextureLoadHelper(const TextureLoadHelper&) = delete;
	void operator=(const TextureLoadHelper&)    = delete;


	       void               SetFlipVerticallyOnLoad(bool);
		   ImageData          LoadImage(const std::string&);
		   void               FreeImage(ImageData&);

		   bool               GetFlipVerticallyOnLoad() const;

	static TextureLoadHelper* GetInstance();
	static void               FreeInstance();

private:

	TextureLoadHelper();

private:

	       bool               m_flipVerticallyOnLoad;

	static TextureLoadHelper* g_instance;
};