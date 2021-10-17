#pragma once

#include <string>

class Texture
{
public:

    Texture(const std::string&);

    unsigned int GetTexture() const;

private:

    unsigned int m_texture;
};