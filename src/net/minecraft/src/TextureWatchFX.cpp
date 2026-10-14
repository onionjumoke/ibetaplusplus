#include "TextureWatchFX.h"

#include <cmath>
#include "Minecraft.h"
#include "Item.h"
#include "World.h"
#include "WorldProvider.h"
#include <algorithm>

TextureWatchFX::TextureWatchFX(Minecraft *minecraft) :
	TextureFX(Item::pocketSundial->getIconFromDamage(0)),
	mc(minecraft),
	compassFrame(0),
	compassAngle(0)
{
	tileImage = 1;
	std::fill(watchIconImageData, watchIconImageData + 256, 0);
	std::fill(dialImageData, dialImageData + 256, 0);
}

void TextureWatchFX::onTick()
{
	double d = 0.0;
	if (mc->theWorld != nullptr && mc->thePlayer != nullptr)
	{
		float f = mc->theWorld->getCelestialAngle(1.0f);
		d = -f * 3.141593f * 2.0f;
		if (mc->theWorld->worldProvider->isNether)
		{
			d = ((double)rand() / RAND_MAX) * 3.1415927410125732 * 2.0;
		}
	}

	double d1;
	for (d1 = d - compassFrame; d1 < -3.1415926535897931; d1 += 6.2831853071795862) {}
	for (; d1 >= 3.1415926535897931; d1 -= 6.2831853071795862) {}
	if (d1 < -1.0) d1 = -1.0;
	if (d1 > 1.0) d1 = 1.0;

	compassAngle += d1 * 0.10000000000000001;
	compassAngle *= 0.80000000000000004;
	compassFrame += compassAngle;

	double d2 = sin(compassFrame);
	double d3 = cos(compassFrame);

	for (int_t i = 0; i < 256; i++)
	{
		int_t j = (watchIconImageData[i] >> 24) & 0xff;
		int_t k = (watchIconImageData[i] >> 16) & 0xff;
		int_t l = (watchIconImageData[i] >> 8) & 0xff;
		int_t i1 = (watchIconImageData[i] >> 0) & 0xff;
		if (k == i1 && l == 0 && i1 > 0)
		{
			double d4 = -((double)(i % 16) / 15.0 - 0.5);
			double d5 = (double)(i / 16) / 15.0 - 0.5;
			int_t i2 = k;
			int_t j2 = (int)((d4 * d3 + d5 * d2 + 0.5) * 16.0);
			int_t k2 = (int)(((d5 * d3 - d4 * d2) + 0.5) * 16.0);
			int_t l2 = (j2 & 0xf) + (k2 & 0xf) * 16;
			j = (dialImageData[l2] >> 24) & 0xff;
			k = ((dialImageData[l2] >> 16) & 0xff) * i2 / 255;
			l = ((dialImageData[l2] >> 8) & 0xff) * i2 / 255;
			i1 = ((dialImageData[l2] >> 0) & 0xff) * i2 / 255;
		}
		if (anaglyphEnabled)
		{
			int_t j1 = (k * 30 + l * 59 + i1 * 11) / 100;
			int_t k1 = (k * 30 + l * 70) / 100;
			int_t l1 = (k * 30 + i1 * 70) / 100;
			k = j1;
			l = k1;
			i1 = l1;
		}
		imageData[i * 4 + 0] = (byte_t)k;
		imageData[i * 4 + 1] = (byte_t)l;
		imageData[i * 4 + 2] = (byte_t)i1;
		imageData[i * 4 + 3] = (byte_t)j;
	}
}
