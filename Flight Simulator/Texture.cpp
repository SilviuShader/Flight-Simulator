#include <iostream>

#include "Shader.h"
#include "glad/glad.h"
#include "Texture.h"

#include "TextureLoadHelper.h"
#include <cassert>

using namespace std;

unordered_map<string, pair<Texture::TextureInfo, int>> Texture::g_texturesCache = unordered_map<string, pair<Texture::TextureInfo, int>>();

Texture::Texture(const string& filename)
{
    memset(&m_textureInfo, 0, sizeof(m_textureInfo));
    m_path = filename;

    TextureInfo textureInfo;
    int         referencesCount;

    if (g_texturesCache.find(m_path) == g_texturesCache.end())
    {
        glGenTextures(1, &textureInfo.TextureID);
        glBindTexture(GL_TEXTURE_2D, textureInfo.TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        TextureLoadHelper::ImageData imageData = TextureLoadHelper::GetInstance()->LoadImage(filename);

        textureInfo.Width = imageData.Width;
        textureInfo.Height = imageData.Height;

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

        referencesCount = 0;
    }
    else
    {
        textureInfo     = g_texturesCache[m_path].first;
        referencesCount = g_texturesCache[m_path].second;
    }

    g_texturesCache[m_path] = make_pair(textureInfo, referencesCount);
}

Texture::Texture(unsigned int textureID) :
    m_textureInfo { textureID, -1, -1 },
    m_path("")
{
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH,  &m_textureInfo.Width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &m_textureInfo.Height);
}

Texture::Texture(int width, int height, Format internalFormat, Format format, Filter filter, float* texData)
{
    m_textureInfo.Width  = width;
    m_textureInfo.Height = height;

    glGenTextures(1, &m_textureInfo.TextureID);
    glBindTexture(GL_TEXTURE_2D, m_textureInfo.TextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParam(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParam(filter));

    glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(internalFormat), width, height, 0, GetGLFormat(format), GL_FLOAT, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture()
{
    if (m_path.size())
    {
        TextureInfo info    = g_texturesCache[m_path].first;
        int referencesCount = g_texturesCache[m_path].second;

        referencesCount--;
        if (referencesCount <= 0)
        {
            glDeleteTextures(1, &info.TextureID);
            g_texturesCache.erase(m_path);
        }
        else
        {
            g_texturesCache[m_path] = make_pair(info, referencesCount);
        }
    }
    else
    {
        glDeleteTextures(1, &m_textureInfo.TextureID);
    }
}

unsigned int Texture::GetTextureID() const
{
    return GetCurrentTextureInfo().TextureID;
}

int Texture::GetWidth() const
{
    return GetCurrentTextureInfo().Width;
}

int Texture::GetHeight() const
{
    return GetCurrentTextureInfo().Height;
}

float** Texture::GetDownscaleValues(DownscaleShaderProperties shaderProps, int levels)
{
    int divisionsCount  = 1 << (levels - 1);
    int div2            = GetWidth() / divisionsCount;

    assert(GetWidth() == GetHeight() &&
           "The texture must have the width equal to the height.");

    assert(GetWidth() > divisionsCount && divisionsCount > 0 &&
        "The width of the noise texture must be greater than the width of the texture containing the downscaled image.");

    assert(GetWidth() % levels == 0 && ((div2 & (div2 - 1)) == 0) &&
        "Texture width has to be equal to levels * pow(2, k), where k > 1.");

    Texture* currentTexture = this;
    Texture* newTexture     = nullptr;
    
    int      itersCount     = log2(GetWidth() / divisionsCount);

    int      add            = 0;
    int      iter           = add;
    do
    {
        add = (iter + shaderProps.ShaderSteps > itersCount) ? itersCount - iter : shaderProps.ShaderSteps;
        iter += add;

        if (!newTexture)
            newTexture = new Texture(GetWidth()  >> iter,
                                     GetHeight() >> iter,
                                     Texture::Format::R32F,
                                     Texture::Format::RED,
                                     Texture::Filter::Linear);

        shaderProps.Shader->Use();
        shaderProps.Shader->SetImage2D("ImageInput",  currentTexture, 0, Texture::Format::R32F);
        shaderProps.Shader->SetImage2D("ImageOutput", newTexture,     1, Texture::Format::R32F);
        shaderProps.Shader->SetInt("Iterations",      add);

        glDispatchCompute(GetComputeShaderGroupsCount(currentTexture->GetWidth()  >> 1, shaderProps.BlocksCount),
                          GetComputeShaderGroupsCount(currentTexture->GetHeight() >> 1, shaderProps.BlocksCount), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        if (currentTexture && currentTexture != this)
        {
            delete currentTexture;
            currentTexture = nullptr;
        }

        currentTexture = newTexture;
        newTexture = nullptr;
    } while (iter < itersCount);

    float** result = GetPixelsInfo(currentTexture);

    if (currentTexture && currentTexture != this)
    {
        delete currentTexture;
        currentTexture = nullptr;
    }

    return result;
}

float** Texture::GetPixelsInfo(Texture* texture)
{
    int    textureWidth  = texture->GetWidth();
    int    textureHeight = texture->GetHeight();
    float* pixels        = new float[textureWidth * textureHeight];

    float** result       = new float*[textureWidth];

    for (int i = 0; i < textureWidth; i++)
        result[i] = new float[textureHeight];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels);

    for (int width = 0; width < textureWidth; width++)
        for (int height = 0; height < textureHeight; height++)
            result[width][height] = pixels[height * textureWidth + width];

    if (pixels)
    {
        delete[] pixels;
        pixels = nullptr;
    }

    return result;
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
    case Format::R8:
        return GL_R8;
    case Format::R32F:
        return GL_R32F;
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

uint32_t Texture::GetComputeShaderGroupsCount(const uint32_t size, const uint32_t numBlocks)
{
    return (size + numBlocks - 1) / numBlocks;
}

unsigned int Texture::CreateMinMaxBuffer()
{
    unsigned int minMaxBuffer;

    MinMaxValues data;
    data.Mn = MIN_MAX_BUFFER_VALUE;
    data.Mx = 0;

    glGenBuffers(1, &minMaxBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, minMaxBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

    return minMaxBuffer;
}

void Texture::FreeMinMaxBuffer(unsigned int minMaxBuffer)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &minMaxBuffer);
}

Texture::TextureInfo Texture::GetCurrentTextureInfo() const
{
    TextureInfo result = m_textureInfo;
    if (m_path.size())
        result = g_texturesCache[m_path].first;

    return result;
}