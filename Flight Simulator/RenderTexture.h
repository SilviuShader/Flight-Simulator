#pragma once

#include "Texture.h"

class RenderTexture
{
public:

    RenderTexture(int, int);
    ~RenderTexture();

    void     Begin();
    Texture* GetTexture()      const;
    Texture* GetDepthTexture() const;

private:

    unsigned int m_frameBuffer;
    unsigned int m_rboDepthId;

    Texture*     m_texture;
    Texture*     m_depthTexture;

    int          m_width;
    int          m_height;
};