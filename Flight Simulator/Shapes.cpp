#include "glad/glad.h"
#include "Shapes.h"
#include <glm/ext/matrix_transform.hpp>
#include "VertexTypes.h"

using namespace glm;

Shapes* Shapes::g_instance = nullptr;

Shapes::~Shapes()
{
	if (m_colorShader)
	{
		delete m_colorShader;
		m_colorShader = nullptr;
	}

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

void Shapes::DrawRectangle(const vec3& center, const vec3& extents, Camera* camera)
{
	mat4 model = translate(mat4(1.0), center) * scale(mat4(1.0), extents);
	mat4 view = camera->GetViewMatrix();
	mat4 projection = camera->GetProjectionMatrix();

	m_colorShader->SetMatrix4("Model",      model);
	m_colorShader->SetMatrix4("View",       view);
	m_colorShader->SetMatrix4("Projection", projection);

	m_colorShader->Use();

	glLineWidth(1.0);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
}

Shapes::Shapes()
{
	CreateCubeBuffers();

	m_colorShader = new Shader("Shaders/Color.vert", "Shaders/Color.frag");
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

	glGenBuffers(1, &m_ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Shapes::FreeCubeBuffers()
{
	glBindVertexArray(m_vao);

	VertexPositionColor::ResetLayout();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_vbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_ebo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_vao);
}
