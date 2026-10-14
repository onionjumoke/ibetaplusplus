#pragma once

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.StepSound
class StepSound
{
public:
	StepSound(const jstring &s, float f, float f1);
	virtual ~StepSound() = default;

	virtual float getVolume();
	virtual float getPitch();
	virtual jstring stepSoundDir();
	virtual jstring getBreakSound();

	const jstring motionZ;
	const float motionY;
	const float rotationYaw;
};
