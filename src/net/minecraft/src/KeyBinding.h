#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.KeyBinding
class KeyBinding
{
public:
	KeyBinding(const std::string &s, int_t i);

	std::string keyDescription;
	int_t keyCode;
};
