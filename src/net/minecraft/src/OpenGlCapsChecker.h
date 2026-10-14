#pragma once

// net.minecraft.src.OpenGlCapsChecker
class OpenGlCapsChecker
{
public:
	OpenGlCapsChecker();

	bool checkARBOcclusion();

private:
	static bool tryCheckOcclusionCapable;
};
