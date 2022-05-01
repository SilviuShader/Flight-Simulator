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

    struct MinMaxValues
    {
    public:

        int Mn;
        int Mx;
    };

public:

    static const int MIN_MAX_BUFFER_VALUE = 100000000;

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

    static unsigned int CreateMinMaxBuffer();
    static void         FreeMinMaxBuffer(unsigned int);

private:

    TextureInfo GetCurrentTextureInfo() const;

private:

           std::string                                                  m_path;
           TextureInfo                                                  m_textureInfo;

    static std::unordered_map<std::string, std::pair<TextureInfo, int>> g_texturesCache;
};