#include "glad/glad.h"
#include "Shapes.h"
#include <glm/ext/matrix_transform.hpp>
#include "VertexTypes.h"
#include "ShaderManager.h"

using namespace glm;

Shapes* Shapes::g_instance = nullptr;

Shapes::~Shapes()
{
	FreeCubeBuffers();
}

Shapes* Shapes::GetInstance()
{
	if (!g_instance)
		g_instance = new Shapes();

	return g_instance;
}

void Shapes::FreeInstance()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance = nullptr;
	}
}

void Shapes::ResetInstances()
{
	m_instances.clear();
}

void Shapes::AddInstance(const vec3& center, const vec3& extents)
{
	m_instances.push_back(translate(mat4(1.0), center) * scale(mat4(1.0), extents));
}

void Shapes::DrawRectangles(Camera* camera)
{
	ShaderManager* shaderManager = ShaderManager::GetInstance();
	Shader*        colorShader   = shaderManager->GetColorShader();

	glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * m_instances.size(), ((m_instances.size()) ? &m_instances[0] : NULL), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	mat4 view       = camera->GetViewMatrix();
	mat4 projection = camera->GetProjectionMatrix();

	colorShader->Use();

	colorShader->SetMatrix4("View",       view);
	colorShader->SetMatrix4("Projection", projection);

	glLineWidth(1.0);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, m_instances.size());
}

Shapes::Shapes()
{
	CreateCubeBuffers();
}

void Shapes::CreateCubeBuffers()
{
	VertexPositionColor vertices[] =
	{
		VertexPositionColor(vec3(-1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-front
		VertexPositionColor(vec3( 1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-front
		VertexPositionColor(vec3( 1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-back
		VertexPositionColor(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-back

		VertexPositionColor(vec3(-1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-top-front
		VertexPositionColor(vec3( 1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-front
		VertexPositionColor(vec3( 1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-back
		VertexPositionColor(vec3(-1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f))  // left-top-back
	};

	unsigned int indices[] =
	{
		// bottom face
		0, 1, 
		1, 2,
		2, 3,
		3, 0,

		// vertical lines
		0, 4,
		1, 5,
		2, 6,
		3, 7,

		// top face
		4, 5,
		5, 6,
		6, 7,
		7, 4
	};

	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	VertexPositionColor::SetLayout();

	glGenBuffers(1, &m_instanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4) * 2));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void*)(sizeof(vec4) * 3));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glGenBuffers(1, &m_ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Shapes::FreeCubeBuffers()
{
	glBindVertexArray(m_vao);

	VertexPositionColor::ResetLayout();

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_instanceVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_ebo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_vao);
}
