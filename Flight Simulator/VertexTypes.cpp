#include "glad/glad.h"
#include "VertexTypes.h"

using namespace glm;

VertexPosition::VertexPosition(vec3 const& position) noexcept :
	Position(position)
{
}

void VertexPosition::SetLayout()
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPosition), (void*)0);
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
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), (void*)0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), (void*)offsetof(VertexPositionColor, Color));
}

void VertexPositionColor::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

VertexPositionTexture::VertexPositionTexture(vec3 const& position, vec2 const& texCoords) noexcept :
	Position(position),
	TexCoords(texCoords)
{
}

void VertexPositionTexture::SetLayout()
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionTexture), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPositionTexture), (void*)offsetof(VertexPositionTexture, TexCoords));
}

void VertexPositionTexture::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

VertexNormalTexture::VertexNormalTexture(vec3 const& position, vec3 const& normal, vec2 const& texCoords) noexcept :
	Position(position),
	Normal(normal),
	TexCoords(texCoords)
{
}

void VertexNormalTexture::SetLayout()
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTexture), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTexture), (void*)offsetof(VertexNormalTexture, Normal));
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTexture), (void*)offsetof(VertexNormalTexture, TexCoords));
}

void VertexNormalTexture::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

VertexNormalTextureBinormalTangent::VertexNormalTextureBinormalTangent(vec3 const& position, vec3 const& normal, vec2 const& texCoords, vec3 const& binormal, vec3 const& tangent) noexcept :
	Position(position),
	Normal(normal),
	TexCoords(texCoords),
	Binormal(binormal),
	Tangent(tangent)
{
}

void VertexNormalTextureBinormalTangent::SetLayout()
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextureBinormalTangent), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextureBinormalTangent), (void*)offsetof(VertexNormalTextureBinormalTangent, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextureBinormalTangent), (void*)offsetof(VertexNormalTextureBinormalTangent, TexCoords));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextureBinormalTangent), (void*)offsetof(VertexNormalTextureBinormalTangent, Binormal));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalTextureBinormalTangent), (void*)offsetof(VertexNormalTextureBinormalTangent, Tangent));
}

void VertexNormalTextureBinormalTangent::ResetLayout()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}
