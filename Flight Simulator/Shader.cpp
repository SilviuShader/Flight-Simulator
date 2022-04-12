#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "Shader.h"

using namespace std;
using namespace glm;

Shader::Shader(const string vertexPath, const string fragmentPath, 
    const string tessControlPath, const string tessEvaluationPath)
{
    unsigned int vertex, fragment;
    unsigned int tessControl, tessEvaluation;
    bool addedTessControl = false;
    bool addedTessEvaluation = false;
    int success;
    char infoLog[SHADER_COMPILE_LOG_LENGTH];

    string vsString = ReadFile(vertexPath);
    string fsString = ReadFile(fragmentPath);

    const char* vertexShaderSource = vsString.c_str();
    const char* fragmentShaderSource = fsString.c_str();

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    if (tessControlPath.size() > 0)
    {
        string tcsString = ReadFile(tessControlPath);
        const char* tessControlShaderSource = tcsString.c_str();

        tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tessControl, 1, &tessControlShaderSource, NULL);
        glCompileShader(tessControl);

        glGetShaderiv(tessControl, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(tessControl, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
            cout << "ERROR::SHADER::TESS_CONTROL::COMPILATION_FAILED\n" << infoLog << endl;
        }

        addedTessControl = true;
    }

    if (tessEvaluationPath.size() > 0)
    {
        string tesString = ReadFile(tessEvaluationPath);
        const char* tessEvaluationShaderSource = tesString.c_str();

        tessEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tessEvaluation, 1, &tessEvaluationShaderSource, NULL);
        glCompileShader(tessEvaluation);

        glGetShaderiv(tessEvaluation, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(tessEvaluation, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
            cout << "ERROR::SHADER::TESS_EVALUATION::COMPILATION_FAILED\n" << infoLog << endl;
        }

        addedTessEvaluation = true;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertex);
    
    if (addedTessControl)
        glAttachShader(m_programId, tessControl);
    if (addedTessEvaluation)
        glAttachShader(m_programId, tessEvaluation);

    glAttachShader(m_programId, fragment);
    glLinkProgram(m_programId);

    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(m_programId, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (addedTessControl)
        glDeleteShader(tessControl);

    if (addedTessEvaluation)
        glDeleteShader(tessEvaluation);
}

Shader::Shader(const string computePath)
{
    unsigned int compute;
    int success;
    char infoLog[SHADER_COMPILE_LOG_LENGTH];

    string csString = ReadFile(computePath);

    const char* computeShaderSource = csString.c_str();

    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &computeShaderSource, NULL);
    glCompileShader(compute);

    glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(compute, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
        cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << endl;
    }

    m_programId = glCreateProgram();
    glAttachShader(m_programId, compute);
    glLinkProgram(m_programId);

    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(m_programId, SHADER_COMPILE_LOG_LENGTH, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    glDeleteShader(compute);
}

Shader::~Shader()
{
    m_hasUniform.clear();
    m_uniformBlocksIndices.clear();
    m_uniformLocations.clear();

    glDeleteProgram(m_programId);
}

void Shader::Use()
{
    glUseProgram(m_programId);
}

bool Shader::HasUniform(const string& name)
{
    if (m_hasUniform.find(name) == m_hasUniform.end())
    {
        int location           = glGetUniformLocation(m_programId, name.c_str());
            m_hasUniform[name] = location != -1;
    }

    return m_hasUniform[name];
}

bool Shader::HasLightUniforms()
{
    return HasUniform("AmbientColor")   && 
           HasUniform("DiffuseColor")   && 
           HasUniform("LightDirection") && 
           HasUniform("SpecularPower")  && 
           HasUniform("CameraPosition");
}

void Shader::SetBool(const string& name, bool value)
{
    glUniform1i(GetUniformLocation(name), (int)value);
}

void Shader::SetInt(const string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const string& name, const vec2& value)
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void Shader::SetVec3(const string& name, const vec3& value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetVec4(const string& name, const vec4& value)
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::SetMatrix4(const string& name, mat4& value)
{
    float* ptr = value_ptr(value);
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, ptr);
}

void Shader::SetTexture(const string& name, Texture* texture, int textureNumber)
{
    glActiveTexture(GL_TEXTURE0 + textureNumber);
    glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());
    SetInt(name, textureNumber);
}

void Shader::SetTexture3D(const string& name, Texture3D* texture, int textureNumber)
{
    glActiveTexture(GL_TEXTURE0 + textureNumber);
    glBindTexture(GL_TEXTURE_3D, texture->GetTextureID());
    SetInt(name, textureNumber);
}

void Shader::SetCubemap(const string& name, Cubemap* cubemap, int textureNumber)
{
    glActiveTexture(GL_TEXTURE0 + textureNumber);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetTextureID());
    SetInt(name, textureNumber);
}

void Shader::SetImage2D(const string& name, Texture* texture, int textureNumber, Texture::Format format)
{
    glBindImageTexture(textureNumber, texture->GetTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, Texture::GetGLFormat(format));
    SetInt(name, textureNumber);
}

void Shader::SetImage3D(const string& name, Texture3D* texture3D, int textureNumber, Texture::Format format)
{
    glBindImageTexture(textureNumber, texture3D->GetTextureID(), 0, GL_TRUE, 0, GL_READ_WRITE, Texture::GetGLFormat(format));
    SetInt(name, textureNumber);
}

void Shader::SetLight(Camera* camera, Light* light)
{
    SetVec4("AmbientColor",   light->GetAmbientColor());
    SetVec4("DiffuseColor",   light->GetDiffuseColor());
    SetVec3("LightDirection", light->GetLightDirection());
    SetFloat("SpecularPower", light->GetSpecularPower());
    SetVec3("CameraPosition", camera->GetPosition());
}

int Shader::SetMaterials(const string& texturesName, const string& normalTexturesName, const string& specularTexturesName, const vector<Material*>& materials, int startingTextureNumber)
{
    int materialsCount = materials.size();

    int* textureNumbers = new int[materialsCount];
    int* normalTextureNumbers = new int[materialsCount];
    int* specularTextureNumbers = new int[materialsCount];

    for (int i = 0; i < materialsCount; i++)
        textureNumbers[i] = startingTextureNumber++;

    for (int i = 0; i < materialsCount; i++)
        normalTextureNumbers[i] = startingTextureNumber++;

    for (int i = 0; i < materialsCount; i++)
        specularTextureNumbers[i] = startingTextureNumber++;

    for (int i = 0; i < materialsCount; i++)
    {
        glActiveTexture(GL_TEXTURE0 + textureNumbers[i]);
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetTexture()->GetTextureID());

        glActiveTexture(GL_TEXTURE0 + normalTextureNumbers[i]);
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetNormalTexture()->GetTextureID());

        glActiveTexture(GL_TEXTURE0 + specularTextureNumbers[i]);
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetSpecularTexture()->GetTextureID());
    }

    glUniform1iv(GetUniformLocation(texturesName), materialsCount, textureNumbers);
    glUniform1iv(GetUniformLocation(normalTexturesName), materialsCount, normalTextureNumbers);
    glUniform1iv(GetUniformLocation(specularTexturesName), materialsCount, specularTextureNumbers);

    if (specularTextureNumbers)
    {
        delete[] specularTextureNumbers;
        specularTextureNumbers = nullptr;
    }

    if (normalTextureNumbers)
    {
        delete[] normalTextureNumbers;
        normalTextureNumbers = nullptr;
    }

    if (textureNumbers)
    {
        delete[] textureNumbers;
        textureNumbers = nullptr;
    }

    return startingTextureNumber;
}

void Shader::SetUniformBlockBinding(const string& name, int binding)
{
    glUniformBlockBinding(m_programId, GetUniformBlockIndex(name), binding);
}

void Shader::SetShaderStorageBlockBinding(const string& name, int binding)
{
    glShaderStorageBlockBinding(m_programId, GetShaderStorageBlockIndex(name), binding);
}

string Shader::ReadFile(const string filename)
{
    try
    {
        ifstream t(filename);
        string str((istreambuf_iterator<char>(t)),
            istreambuf_iterator<char>());

        return str;
    }
    catch (ifstream::failure e)
    {
        cout << "ERROR::FILE::NOT_SUCCESFULLY_READ" << endl;
    }

    return "";
}

int Shader::GetUniformLocation(const string& name)
{
    if (m_uniformLocations.find(name) != m_uniformLocations.end())
        return m_uniformLocations[name];

    int location = glGetUniformLocation(m_programId, name.c_str());
    
    if (location == -1)
    {
        cout << "ERROR::SHADER::UNIFORM::LOCATION::NOT_FOUND" << endl;
        return location;
    }

    m_uniformLocations[name] = location;
    return location;
}

int Shader::GetUniformBlockIndex(const string& name)
{
    if (m_uniformBlocksIndices.find(name) != m_uniformBlocksIndices.end())
        return m_uniformBlocksIndices[name];

    int index = glGetUniformBlockIndex(m_programId, name.c_str());

    if (index == GL_INVALID_INDEX)
    {
        cout << "ERROR::SHADER::UNIFORM::BLOCK_INDEX::NOT_FOUND" << endl;
        return index;
    }

    m_uniformBlocksIndices[name] = index;
    return index;
}

int Shader::GetShaderStorageBlockIndex(const string& name)
{
    if (m_shaderStorageBlocksIndices.find(name) != m_shaderStorageBlocksIndices.end())
        return m_shaderStorageBlocksIndices[name];

    int index = glGetProgramResourceIndex(m_programId, GL_SHADER_STORAGE_BLOCK, name.c_str());

    if (index == GL_INVALID_INDEX)
    {
        cout << "ERROR::SHADER::UNIFORM::BLOCK_INDEX::NOT_FOUND" << endl;
        return index;
    }

    m_shaderStorageBlocksIndices[name] = index;
    return index;
}
