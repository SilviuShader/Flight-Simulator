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
}

void Shader::Use()
{
    glUseProgram(m_programId);
}

void Shader::SetBool(const string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), (int)value);
}

void Shader::SetInt(const string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::SetFloat(const string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::SetVec2(const string& name, const vec2& value) const
{
    glUniform2f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y);
}

void Shader::SetVec3(const string& name, const vec3& value) const
{
    glUniform3f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y, value.z);
}

void Shader::SetVec4(const string& name, const vec4& value) const
{
    glUniform4f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::SetMatrix4(const string& name, mat4& value) const
{
    float* ptr = value_ptr(value);
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, ptr);
}

void Shader::SetBlockBinding(const string& name, int binding) const
{
    glUniformBlockBinding(m_programId, glGetUniformBlockIndex(m_programId, name.c_str()), binding);
}

void Shader::SetTexture(const string& name, Texture* texture, int textureNumber) const
{
    glActiveTexture(GL_TEXTURE0 + textureNumber);
    glBindTexture(GL_TEXTURE_2D, texture->GetTexture());
    SetInt(name, textureNumber);
}

int Shader::SetMaterials(const string& texturesName, const string& normalTexturesName, const string& specularTexturesName, const vector<Material*>& materials, int startingTextureNumber) const
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
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetTexture()->GetTexture());

        glActiveTexture(GL_TEXTURE0 + normalTextureNumbers[i]);
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetNormalTexture()->GetTexture());

        glActiveTexture(GL_TEXTURE0 + specularTextureNumbers[i]);
        glBindTexture(GL_TEXTURE_2D, materials[i]->GetSpecularTexture()->GetTexture());
    }

    glUniform1iv(glGetUniformLocation(m_programId, texturesName.c_str()), materialsCount, textureNumbers);
    glUniform1iv(glGetUniformLocation(m_programId, normalTexturesName.c_str()), materialsCount, normalTextureNumbers);
    glUniform1iv(glGetUniformLocation(m_programId, specularTexturesName.c_str()), materialsCount, specularTextureNumbers);

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
