#pragma once

#include "Texture.h"

class RenderTexture
{
public:

    RenderTexture(int, int);
    ~RenderTexture();

    void     Begin();
    Texture* GetTexture() const;

private:

    unsigned int m_frameBuffer;
    Texture*     m_texture;

    int          m_width;
    int          m_height;
};