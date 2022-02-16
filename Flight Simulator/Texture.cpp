#include <iostream>

#include "glad/glad.h"
#include "Texture.h"

#include "TextureLoadHelper.h"

using namespace std;

Texture::Texture(const string& filename)
{
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TextureLoadHelper::ImageData imageData = TextureLoadHelper::GetInstance()->LoadImage(filename);
    
    m_width  = imageData.Width;
    m_height = imageData.Height;

    if (imageData.Data)
    {
        int internalFormat = GL_RGB;
        if (imageData.ChannelsCount == 4)
            internalFormat = GL_RGBA;
        if (imageData.ChannelsCount == 1)
            internalFormat = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageData.Width, imageData.Height, 0, internalFormat, GL_UNSIGNED_BYTE, imageData.Data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture: " << filename << endl;
    }

    TextureLoadHelper::GetInstance()->FreeImage(imageData);
}

Texture::Texture(unsigned int textureID) :
    m_textureID(textureID),
    m_width(-1),
    m_height(-1)
{
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH,  &m_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &m_height);
}

Texture::Texture(int width, int height, Format internalFormat, Format format, Filter filter, float* texData)
{
    m_width = width;
    m_height = height;

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParam(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParam(filter));

    glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(internalFormat), width, height, 0, GetGLFormat(format), GL_FLOAT, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_textureID);
}

unsigned int Texture::GetTextureID() const
{
    return m_textureID;
}

int Texture::GetWidth() const
{
    return m_width;
}

int Texture::GetHeight() const
{
    return m_height;
}

int Texture::GetGLFormat(Format format)
{
    switch (format)
    {
    case Format::RGBA32F:
        return GL_RGBA32F;
    case Format::RGBA:
        return GL_RGBA;
    case Format::RED:
        return GL_RED;
    }

    cout << "ERROR::TEXTURE::INVALID::FORMAT" << endl;

    return -1;
}

int Texture::GetGLParam(Filter filter)
{
    switch (filter)
    {
    case Filter::Linear:
        return GL_LINEAR;
    case Filter::Point:
        return GL_NEAREST;
    }

    cout << "ERROR::TEXTURE::INVALID::FILTER" << endl;

    return -1;
}