#pragma once

#include <string>
#include <vector>

#include "Texture.h"
#include "Cubemap.h"
#include "Material.h"

class Shader
{
private:

    static const int SHADER_COMPILE_LOG_LENGTH = 512;

public:

    Shader(const std::string, const std::string, const std::string = "", const std::string = "");
    Shader(const std::string);

    void Use();

    void SetBool(const std::string&, bool)                                                                            const;
    void SetInt(const std::string&, int)                                                                              const;
    void SetFloat(const std::string&, float)                                                                          const;
    void SetVec2(const std::string&, const glm::vec2&)                                                                const;
    void SetVec3(const std::string&, const glm::vec3&)                                                                const;
    void SetVec4(const std::string&, const glm::vec4&)                                                                const;
    void SetMatrix4(const std::string&, glm::mat4&)                                                                   const;
    void SetBlockBinding(const std::string&, int)                                                                     const;
    void SetTexture(const std::string&, Texture*, int)                                                                const;
    void SetCubemap(const std::string&, Cubemap*, int)                                                                const;
    void SetImage2D(const std::string&, Texture*, int)                                                                const;
    int  SetMaterials(const std::string&, const std::string&, const std::string&, const std::vector<Material*>&, int) const;

private:

    std::string ReadFile(const std::string);

private:

    int m_programId;
};