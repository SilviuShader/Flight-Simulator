#pragma once

#include <string>

class Texture
{
public:

    Texture(const std::string&);
    Texture(unsigned int);
    ~Texture();

    unsigned int GetTexture() const;

private:

    unsigned int m_texture;
};