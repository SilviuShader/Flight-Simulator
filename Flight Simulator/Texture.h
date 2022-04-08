#pragma once

#include <string>
#include <unordered_map>

class Shader;

class Texture
{
public:

    enum class Format
    {
        RGBA32F,
        RGBA,
        RED,
        R8,
        R32F
    };

    enum class Filter
    {
        Linear,
        Point
    };

    struct DownscaleShaderProperties
    {
        Shader* Shader;
        int     ShaderSteps;
        int     BlocksCount;
    };

private:

    struct TextureInfo
    {
    public:

        unsigned int TextureID;
        int          Width;
        int          Height;
    };

public:

    Texture(const std::string&); // load texture from file
    Texture(unsigned int);       // for textures that were already created
    Texture(int, int, 
            Format = Format::RGBA32F,
            Format = Format::RGBA, 
            Filter = Filter::Linear, 
            float*  = NULL);     // for new textures

    ~Texture();

           unsigned int GetTextureID() const;
           
           int          GetWidth()     const;
           int          GetHeight()    const;

           // These two methods only work for grayscale images.
           float**      GetDownscaleValues(DownscaleShaderProperties, int);
    static float**      GetPixelsInfo(Texture*);

    static int          GetGLFormat(Format);
    static int          GetGLParam(Filter);
    static uint32_t     GetComputeShaderGroupsCount(const uint32_t, const uint32_t);

private:

    TextureInfo GetCurrentTextureInfo() const;

private:

           std::string                                                  m_path;
           TextureInfo                                                  m_textureInfo;

    static std::unordered_map<std::string, std::pair<TextureInfo, int>> g_texturesCache;
};