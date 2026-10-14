#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.NibbleArray
class NibbleArray
{
public:
	NibbleArray(int_t i) : data(i >> 1, 0) {}
	NibbleArray(std::vector<byte_t> bytes) : data(std::move(bytes)) {}

	int_t getNibble(int_t i, int_t j, int_t k)
	{
		i &= 15;
		k &= 15;
		j &= 127;
		int_t l = i << 11 | k << 7 | j;
		int_t i1 = l >> 1;
		if (i1 >= (int_t)data.size())
			return 0; // short/garbage array (MP packet, corrupt save)
		int_t j1 = l & 1;
		if (j1 == 0)
			return data[i1] & 0xf;
		else
			return (data[i1] >> 4) & 0xf;
	}

	void setNibble(int_t i, int_t j, int_t k, int_t l)
	{
		i &= 15;
		k &= 15;
		j &= 127;
		int_t i1 = i << 11 | k << 7 | j;
		int_t j1 = i1 >> 1;
		if (j1 >= (int_t)data.size())
			return;
		int_t k1 = i1 & 1;
		if (k1 == 0)
			data[j1] = (byte_t)(data[j1] & 0xf0 | l & 0xf);
		else
			data[j1] = (byte_t)(data[j1] & 0xf | (l & 0xf) << 4);
	}

	bool isValid() const { return !data.empty(); }

	std::vector<byte_t> data;
};
