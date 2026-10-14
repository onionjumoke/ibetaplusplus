#include "TextureFlamesFX.h"

#include <cmath>
#include "Block.h"
#include "BlockFire.h"

TextureFlamesFX::TextureFlamesFX(int_t i) :
	TextureFX(0)
{
	iconIndex = Block::fire->blockIndexInTexture + i * 16;
	for (int_t j = 0; j < 320; j++)
	{
		currentFrame[j] = 0.0f;
		nextFrame[j] = 0.0f;
	}
}

void TextureFlamesFX::onTick()
{
	for (int_t i = 0; i < 16; i++)
	{
		for (int_t j = 0; j < 20; j++)
		{
			int_t l = 18;
			float f1 = currentFrame[i + ((j + 1) % 20) * 16] * (float)l;
			for (int_t i1 = i - 1; i1 <= i + 1; i1++)
			{
				for (int_t k1 = j; k1 <= j + 1; k1++)
				{
					int_t i2 = i1;
					int_t k2 = k1;
					if (i2 >= 0 && k2 >= 0 && i2 < 16 && k2 < 20)
					{
						f1 += currentFrame[i2 + k2 * 16];
					}
					l++;
				}
			}
			nextFrame[i + j * 16] = f1 / ((float)l * 1.06f);
			if (j >= 19)
			{
				nextFrame[i + j * 16] = (float)(((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX) * 4.0 + ((double)rand() / RAND_MAX) * 0.10000000149011612 + 0.20000000298023224);
			}
		}
	}

	std::swap(nextFrame, currentFrame);

	for (int_t k = 0; k < 256; k++)
	{
		float f = currentFrame[k] * 1.8f;
		if (f > 1.0f) f = 1.0f;
		if (f < 0.0f) f = 0.0f;
		float f2 = f;
		int_t j1 = (int)(f2 * 155.0f + 100.0f);
		int_t l1 = (int)(f2 * f2 * 255.0f);
		int_t j2 = (int)(f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * f2 * 255.0f);
		byte_t c = (byte_t)0xff;
		if (f2 < 0.5f)
		{
			c = 0;
		}
		f2 = (f2 - 0.5f) * 2.0f;
		if (anaglyphEnabled)
		{
			int_t l2 = (j1 * 30 + l1 * 59 + j2 * 11) / 100;
			int_t i3 = (j1 * 30 + l1 * 70) / 100;
			int_t j3 = (j1 * 30 + j2 * 70) / 100;
			j1 = l2;
			l1 = i3;
			j2 = j3;
		}
		imageData[k * 4 + 0] = (byte_t)j1;
		imageData[k * 4 + 1] = (byte_t)l1;
		imageData[k * 4 + 2] = (byte_t)j2;
		imageData[k * 4 + 3] = c;
	}
}
