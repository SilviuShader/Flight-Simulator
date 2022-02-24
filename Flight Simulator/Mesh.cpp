#include "glad/glad.h"

#include "Mesh.h"

using namespace std;

Mesh::Mesh(vector<VertexNormalTexture> vertices, vector<unsigned int> indices, vector<Material*> materials) :
	m_vertices(vertices),
	m_indices(indices),
	m_materials(materials),
	m_vao(0),
	m_vbo(0),
	m_ebo(0)
{
	SetupMesh();
}

Mesh::~Mesh()
{
    glBindVertexArray(m_vao);

    VertexNormalTexture::ResetLayout();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_ebo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_vao);
}

int Mesh::Draw(Shader* shader, const string& texturesName, const string& normalTexturesName, const string& specularTexturesName, int startingTextureNumber)
{
    int resultTextureNumber = shader->SetMaterials(texturesName, normalTexturesName, specularTexturesName, m_materials, startingTextureNumber);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    return resultTextureNumber;
}

void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VertexNormalTexture), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    VertexNormalTexture::SetLayout();

    glBindVertexArray(0);
}
