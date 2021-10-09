#include <fstream>
#include <iostream>

#include "glad/glad.h"
#include "Shader.h"

using namespace std;

Shader::Shader(const string vertexPath, const string fragmentPath)
{
    unsigned int vertex, fragment;
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

void Shader::SetMatrix4(const string name, float* value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, value);
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
