#include "StepSound.h"

StepSound::StepSound(const jstring &s, float f, float f1) :
	motionZ(s),
	motionY(f),
	rotationYaw(f1)
{
}

float StepSound::getVolume()
{
	return motionY;
}

float StepSound::getPitch()
{
	return rotationYaw;
}

jstring StepSound::stepSoundDir()
{
	return u"step." + motionZ;
}

jstring StepSound::getBreakSound()
{
	return u"step." + motionZ;
}
