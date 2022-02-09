#include <iostream>

#include "glad/glad.h"
#include "Texture.h"

#include "stb_image.h"

using namespace std;

Texture::Texture(const string& filename)
{
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

    m_width  = width;
    m_height = height;

    if (data)
    {
        int internalFormat = GL_RGB;
        if (nrChannels == 4)
            internalFormat = GL_RGBA;
        if (nrChannels == 1)
            internalFormat = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture: " << filename << endl;
    }

    stbi_image_free(data);
}

Texture::Texture(unsigned int textureID) :
    m_textureID(textureID),
    m_width(-1),
    m_height(-1)
{
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &m_width);
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
        break;
    case Format::RGBA:
        return GL_RGBA;
        break;
    case Format::RED:
        return GL_RED;
        break;
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
        break;
    case Filter::Point:
        return GL_NEAREST;
        break;
    }

    cout << "ERROR::TEXTURE::INVALID::FILTER" << endl;

    return -1;
}