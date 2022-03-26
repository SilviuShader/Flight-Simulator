#include "glad/glad.h"

#include "Mesh.h"

using namespace std;
using namespace glm;

Mesh::Mesh(vector<VertexNormalTextureBinormalTangent> vertices, vector<unsigned int> indices, vector<Material*> materials, bool instanced) :
	m_vertices(vertices),
	m_indices(indices),
	m_materials(materials),
	m_vao(0),
	m_vbo(0),
	m_ebo(0),
    m_instanced(instanced),
    m_instancesCount(0)
{
	SetupMesh();
}

Mesh::~Mesh()
{
    glBindVertexArray(m_vao);

    VertexNormalTextureBinormalTangent::ResetLayout();

    if (m_instanced)
    {
        glDisableVertexAttribArray(5);
        glDisableVertexAttribArray(6);
        glDisableVertexAttribArray(7);
        glDisableVertexAttribArray(8);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &m_instanceVbo);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_vao);
}

void Mesh::SetInstances(const vector<mat4>& instances)
{
    if (!m_instanced)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * instances.size(), instances.size() ? &instances[0] : NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_instancesCount = instances.size();
}

int Mesh::Draw(Shader* shader, const string& texturesName, const string& normalTexturesName, const string& specularTexturesName, int startingTextureNumber)
{
    int resultTextureNumber = startingTextureNumber;
    if (shader->HasUniform(texturesName) && shader->HasUniform(normalTexturesName) && shader->HasUniform(specularTexturesName))
    {
        resultTextureNumber = shader->SetMaterials(texturesName, normalTexturesName, specularTexturesName, m_materials, startingTextureNumber);
    }
    else
    {
        if (shader->HasUniform("DiffuseTexture") && m_materials.size() > 0)
        {
            shader->SetTexture("DiffuseTexture", m_materials[0]->GetTexture(), startingTextureNumber);
            resultTextureNumber++;
        }
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    
    if (!m_instanced)
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    else
        glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, m_instancesCount);

    glBindVertexArray(0);

    return resultTextureNumber;
}

std::vector<Material*>& Mesh::GetMaterials()
{
    return m_materials;
}

void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VertexNormalTextureBinormalTangent), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    VertexNormalTextureBinormalTangent::SetLayout();

    if (m_instanced)
    {
        glGenBuffers(1, &m_instanceVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

        int vec4Size = sizeof(vec4);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
    }

    glBindVertexArray(0);
}
