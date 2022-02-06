#include "glad/glad.h"
#include "Shapes.h"
#include <glm/ext/matrix_transform.hpp>

using namespace glm;

Shapes* Shapes::g_instance = nullptr;

Shapes::Vertex::Vertex() :
	Position(0.0f, 0.0f, 0.0f),
	Color(0.0f, 0.0f, 0.0f)
{
}

Shapes::Vertex::Vertex(vec3 position, vec3 color) :
	Position(position),
	Color(color)
{
}

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
	Vertex vertices[] =
	{
		Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-front
		Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-front
		Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-bottom-back
		Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-bottom-back

		Vertex(vec3(-1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // left-top-front
		Vertex(vec3( 1.0f,  1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-front
		Vertex(vec3( 1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)), // right-top-back
		Vertex(vec3(-1.0f,  1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f))  // left-top-back
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &m_ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Shapes::FreeCubeBuffers()
{
	glBindVertexArray(m_vao);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_vbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &m_ebo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &m_vao);
}
