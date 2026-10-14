#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.GLAllocation
class GLAllocation
{
public:
	static int_t generateDisplayLists(int_t i);
	static void generateTextureNames(std::vector<int_t> &intbuffer);
	static void func_28194_b(int_t i);
	static void deleteTexturesAndDisplayLists();
	static std::vector<byte_t> createDirectByteBuffer(int_t i);
	static std::vector<int_t> createDirectIntBuffer(int_t i);
	static std::vector<float> createDirectFloatBuffer(int_t i);

private:
	static std::vector<int_t> displayLists;
	static std::vector<int_t> textureNames;
};
