#include "GLAllocation.h"

#include <algorithm>
#include <stdexcept>
#include "pc/OpenGL.h"

std::vector<int_t> GLAllocation::displayLists;
std::vector<int_t> GLAllocation::textureNames;

int_t GLAllocation::generateDisplayLists(int_t i)
{
	if (i <= 0)
		throw std::invalid_argument("Display-list count must be positive");

	int_t j = (int_t)glGenLists(i);
	if (j == 0)
		throw std::runtime_error("OpenGL could not allocate display lists");

	displayLists.push_back(j);
	displayLists.push_back(i);
	return j;
}

void GLAllocation::generateTextureNames(std::vector<int_t> &intbuffer)
{
	glGenTextures((GLsizei)intbuffer.size(), reinterpret_cast<GLuint *>(intbuffer.data()));
	for (int_t value : intbuffer)
		textureNames.push_back(value);
}

void GLAllocation::func_28194_b(int_t i)
{
	auto it = std::find(displayLists.begin(), displayLists.end(), i);
	if (it == displayLists.end())
		return;
	int_t j = (int_t)(it - displayLists.begin());
	glDeleteLists(displayLists[j], displayLists[j + 1]);
	displayLists.erase(displayLists.begin() + j);
	displayLists.erase(displayLists.begin() + j);
}

void GLAllocation::deleteTexturesAndDisplayLists()
{
	for (int_t i = 0; i < (int_t)displayLists.size(); i += 2)
		glDeleteLists(displayLists[i], displayLists[i + 1]);

	if (!textureNames.empty())
		glDeleteTextures((GLsizei)textureNames.size(), reinterpret_cast<GLuint *>(textureNames.data()));
	displayLists.clear();
	textureNames.clear();
}

std::vector<byte_t> GLAllocation::createDirectByteBuffer(int_t i)
{
	return std::vector<byte_t>(i);
}

std::vector<int_t> GLAllocation::createDirectIntBuffer(int_t i)
{
	return std::vector<int_t>(i);
}

std::vector<float> GLAllocation::createDirectFloatBuffer(int_t i)
{
	return std::vector<float>(i);
}
