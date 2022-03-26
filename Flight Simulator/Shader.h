#pragma once

#include <string>
#include <vector>

#include "Texture.h"
#include "Cubemap.h"
#include "Material.h"
#include <unordered_map>

#include "Light.h"
#include "Camera.h"

class Shader
{
private:

    static const int SHADER_COMPILE_LOG_LENGTH = 512;

public:

    Shader(const std::string, const std::string, const std::string = "", const std::string = "");
    Shader(const std::string);
    ~Shader();

    void Use();

    bool HasUniform(const std::string&);
    bool HasLightUniforms();

    void SetBool(const std::string&, bool);
    void SetInt(const std::string&, int);
    void SetFloat(const std::string&, float);
    void SetVec2(const std::string&, const glm::vec2&);
    void SetVec3(const std::string&, const glm::vec3&);
    void SetVec4(const std::string&, const glm::vec4&);
    void SetMatrix4(const std::string&, glm::mat4&);
    void SetBlockBinding(const std::string&, int);
    void SetTexture(const std::string&, Texture*, int);
    void SetCubemap(const std::string&, Cubemap*, int);
    void SetImage2D(const std::string&, Texture*, int, Texture::Format);
    void SetLight(Camera*, Light*);
    int  SetMaterials(const std::string&, const std::string&, const std::string&, const std::vector<Material*>&, int);

private:

    std::string ReadFile(const std::string);
    int         GetUniformLocation(const std::string&);
    int         GetUniformBlockIndex(const std::string&);

private:

    int                                   m_programId;
    std::unordered_map<std::string, int>  m_uniformLocations;
    std::unordered_map<std::string, int>  m_uniformBlocksIndices;
    std::unordered_map<std::string, bool> m_hasUniform;
};