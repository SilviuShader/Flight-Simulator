#pragma once

#include <string>
#include <vector>

class Cubemap
{
public:

	Cubemap(const std::vector<std::string>&);
	~Cubemap();

	unsigned int GetTextureID() const;

private:

	unsigned int m_textureID;
};