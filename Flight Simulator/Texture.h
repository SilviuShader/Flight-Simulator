#pragma once

#include <string>

class Texture
{
public:

    Texture(const std::string&); // load texture from file
    Texture(unsigned int);       // for textures that were already created
    Texture(float*, int, int);   // for red-only textures
    ~Texture();

    unsigned int GetTexture() const;

    int          GetWidth()   const;
    int          GetHeight()  const;

private:

    unsigned int m_texture;
    int          m_width;
    int          m_height;
};