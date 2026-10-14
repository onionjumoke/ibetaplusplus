#include "TextureCompassFX.h"

#include <cmath>
#include "Minecraft.h"
#include "Item.h"
#include "World.h"
#include "ChunkCoordinates.h"
#include "EntityPlayerSP.h"
#include "WorldProvider.h"
#include "MathHelper.h"
#include <algorithm>

TextureCompassFX::TextureCompassFX(Minecraft *minecraft) :
	TextureFX(Item::compass->getIconFromDamage(0)),
	mc(minecraft),
	watchAngle(0),
	watchFrame(0)
{
	tileImage = 1;
	std::fill(compassIconImageData, compassIconImageData + 256, 0);
}

void TextureCompassFX::onTick()
{
	for (int_t i = 0; i < 256; i++)
	{
		int_t j = (compassIconImageData[i] >> 24) & 0xff;
		int_t k = (compassIconImageData[i] >> 16) & 0xff;
		int_t l = (compassIconImageData[i] >> 8) & 0xff;
		int_t i1 = (compassIconImageData[i] >> 0) & 0xff;
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

	double d = 0.0;
	if (mc->theWorld != nullptr && mc->thePlayer != nullptr)
	{
		ChunkCoordinates chunkcoordinates = mc->theWorld->getSpawnPoint();
		double d2 = (double)chunkcoordinates.x - mc->thePlayer->posX;
		double d4 = (double)chunkcoordinates.z - mc->thePlayer->posZ;
		d = ((double)(mc->thePlayer->rotationYaw - 90.0f) * 3.1415926535897931) / 180.0 - atan2(d4, d2);
		if (mc->theWorld->worldProvider->isNether)
		{
			d = ((double)rand() / RAND_MAX) * 3.1415927410125732 * 2.0;
		}
	}

	double d1;
	for (d1 = d - watchAngle; d1 < -3.1415926535897931; d1 += 6.2831853071795862) {}
	for (; d1 >= 3.1415926535897931; d1 -= 6.2831853071795862) {}
	if (d1 < -1.0) d1 = -1.0;
	if (d1 > 1.0) d1 = 1.0;

	watchFrame += d1 * 0.10000000000000001;
	watchFrame *= 0.80000000000000004;
	watchAngle += watchFrame;

	double d3 = sin(watchAngle);
	double d5 = cos(watchAngle);

	// Draw needle
	for (int_t i2 = -4; i2 <= 4; i2++)
	{
		int_t k2 = (int)(8.5 + d5 * (double)i2 * 0.29999999999999999);
		int_t i3 = (int)(7.5 - d3 * (double)i2 * 0.29999999999999999 * 0.5);
		int_t k3 = i3 * 16 + k2;
		int_t i4 = 100;
		int_t k4 = 100;
		int_t i5 = 100;
		byte_t c = (byte_t)0xff;
		if (anaglyphEnabled)
		{
			int_t k5 = (i4 * 30 + k4 * 59 + i5 * 11) / 100;
			int_t i6 = (i4 * 30 + k4 * 70) / 100;
			int_t k6 = (i4 * 30 + i5 * 70) / 100;
			i4 = k5;
			k4 = i6;
			i5 = k6;
		}
		imageData[k3 * 4 + 0] = (byte_t)i4;
		imageData[k3 * 4 + 1] = (byte_t)k4;
		imageData[k3 * 4 + 2] = (byte_t)i5;
		imageData[k3 * 4 + 3] = c;
	}

	for (int_t j2 = -8; j2 <= 16; j2++)
	{
		int_t l2 = (int)(8.5 + d3 * (double)j2 * 0.29999999999999999);
		int_t j3 = (int)(7.5 + d5 * (double)j2 * 0.29999999999999999 * 0.5);
		int_t l3 = j3 * 16 + l2;
		int_t j4 = j2 < 0 ? 100 : 255;
		int_t l4 = j2 < 0 ? 100 : 20;
		int_t j5 = j2 < 0 ? 100 : 20;
		byte_t c1 = (byte_t)0xff;
		if (anaglyphEnabled)
		{
			int_t l5 = (j4 * 30 + l4 * 59 + j5 * 11) / 100;
			int_t j6 = (j4 * 30 + l4 * 70) / 100;
			int_t l6 = (j4 * 30 + j5 * 70) / 100;
			j4 = l5;
			l4 = j6;
			j5 = l6;
		}
		imageData[l3 * 4 + 0] = (byte_t)j4;
		imageData[l3 * 4 + 1] = (byte_t)l4;
		imageData[l3 * 4 + 2] = (byte_t)j5;
		imageData[l3 * 4 + 3] = c1;
	}
}
