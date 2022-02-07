#include "glad/glad.h"
#include "VertexTypes.h"

using namespace glm;

VertexPosition::VertexPosition(vec3 const& position) noexcept :
	Position(position)
{
}

void VertexPosition::SetLayout()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPosition), (void*)0);
	glEnableVertexAttribArray(0);
}

void VertexPosition::ResetLayout()
{
	glDisableVertexAttribArray(0);
}


VertexPositionColor::VertexPositionColor(vec3 const& position, vec3 const& color) noexcept :
	Position(position),
	Color(color)
{
}

void VertexPositionColor::SetLayout()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), (void*)(sizeof(vec3)));
	glEnableVertexAttribArray(1);
}

void VertexPositionColor::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

VertexPositionTexture::VertexPositionTexture(vec3 const& position, vec2 const& texCoord) noexcept :
	Position(position),
	TexCoord(texCoord)
{
}

void VertexPositionTexture::SetLayout()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionTexture), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPositionTexture), (void*)(sizeof(vec3)));
	glEnableVertexAttribArray(1);
}

void VertexPositionTexture::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}