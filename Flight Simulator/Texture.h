#pragma once

#include <string>

class Texture
{
public:

    enum class Format
    {
        RGBA32F,
        RGBA,
        RED
    };

    enum class Filter
    {
        Linear,
        Point
    };

public:

    Texture(const std::string&);                         // load texture from file
    Texture(unsigned int);                               // for textures that were already created
    Texture(int, int, 
            Format = Format::RGBA32F, 
            Format = Format::RGBA, 
            Filter = Filter::Linear, 
            float* = NULL);                              // for new textures

    ~Texture();

    unsigned int GetTextureID() const;

    int          GetWidth()     const;
    int          GetHeight()    const;

private:

    int GetGLFormat(Format);
    int GetGLParam(Filter);

private:

    unsigned int m_textureID;
    int          m_width;
    int          m_height;
};