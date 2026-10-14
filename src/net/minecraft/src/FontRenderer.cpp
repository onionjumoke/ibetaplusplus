#include "FontRenderer.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "ChatAllowedCharacters.h"
#include "GLAllocation.h"
#include "GameSettings.h"
#include "RenderEngine.h"
#include "Tessellator.h"
#include "pc/OpenGL.h"

FontRenderer::FontRenderer(GameSettings *gamesettings, const std::string &s, RenderEngine *renderengine)
	: fontTextureName(0)
	, fontDisplayLists(0)
{
	for (int_t i = 0; i < 256; i++)
		charWidth[i] = 0;
	buffer.reserve(1024);

	std::vector<int_t> pixels = renderengine->func_28149_a(s);
	int_t width = 128;
	if ((int_t)pixels.size() < 128 * 128)
	{
		for (int_t k = 0; k < 256; k++)
			charWidth[k] = k == 32 ? 4 : 8;
	}
	else
	{
		for (int_t k = 0; k < 256; k++)
		{
			int_t l = k % 16;
			int_t k1 = k / 16;
			int_t j2 = 7;
			for (; j2 >= 0; j2--)
			{
				int_t i3 = l * 8 + j2;
				bool flag = true;
				for (int_t l3 = 0; l3 < 8 && flag; l3++)
				{
					int_t i4 = (k1 * 8 + l3) * width;
					int_t alpha = (pixels[i3 + i4] >> 24) & 0xff;
					int_t blue = pixels[i3 + i4] & 0xff;
					if (alpha > 0 && blue > 0)
						flag = false;
				}
				if (!flag)
					break;
			}
			if (k == 32)
				j2 = 2;
			charWidth[k] = j2 + 2;
		}
	}

	fontTextureName = renderengine->getTexture(s);
	fontDisplayLists = GLAllocation::generateDisplayLists(288);
	Tessellator *tessellator = &Tessellator::instance;
	for (int_t i1 = 0; i1 < 256; i1++)
	{
		glNewList(fontDisplayLists + i1, GL_COMPILE);
		tessellator->startDrawingQuads();
		int_t l1 = (i1 % 16) * 8;
		int_t k2 = (i1 / 16) * 8;
		float f = 7.99f;
		float f1 = 0.0f;
		float f2 = 0.0f;
		tessellator->addVertexWithUV(0.0, 0.0f + f, 0.0, (float)l1 / 128.0f + f1, ((float)k2 + f) / 128.0f + f2);
		tessellator->addVertexWithUV(0.0f + f, 0.0f + f, 0.0, ((float)l1 + f) / 128.0f + f1, ((float)k2 + f) / 128.0f + f2);
		tessellator->addVertexWithUV(0.0f + f, 0.0, 0.0, ((float)l1 + f) / 128.0f + f1, (float)k2 / 128.0f + f2);
		tessellator->addVertexWithUV(0.0, 0.0, 0.0, (float)l1 / 128.0f + f1, (float)k2 / 128.0f + f2);
		tessellator->draw();
		glTranslatef((float)charWidth[i1], 0.0f, 0.0f);
		glEndList();
	}

	for (int_t j1 = 0; j1 < 32; j1++)
	{
		int_t i2 = (j1 >> 3 & 1) * 85;
		int_t l2 = (j1 >> 2 & 1) * 170 + i2;
		int_t j3 = (j1 >> 1 & 1) * 170 + i2;
		int_t k3 = (j1 >> 0 & 1) * 170 + i2;
		if (j1 == 6)
			l2 += 85;
		bool flag1 = j1 >= 16;
		if (gamesettings != nullptr && gamesettings->anaglyph)
		{
			int_t j4 = (l2 * 30 + j3 * 59 + k3 * 11) / 100;
			int_t l4 = (l2 * 30 + j3 * 70) / 100;
			int_t i5 = (l2 * 30 + k3 * 70) / 100;
			l2 = j4;
			j3 = l4;
			k3 = i5;
		}
		if (flag1)
		{
			l2 /= 4;
			j3 /= 4;
			k3 /= 4;
		}
		colorCodeR[j1] = l2;
		colorCodeG[j1] = j3;
		colorCodeB[j1] = k3;
		glNewList(fontDisplayLists + 256 + j1, GL_COMPILE);
		glColor3f((float)l2 / 255.0f, (float)j3 / 255.0f, (float)k3 / 255.0f);
		glEndList();
	}
}

void FontRenderer::drawStringWithShadow(const std::string &s, int_t i, int_t j, int_t k)
{
	renderString(s, i + 1, j + 1, k, true);
	drawString(s, i, j, k);
}

void FontRenderer::drawString(const std::string &s, int_t i, int_t j, int_t k)
{
	renderString(s, i, j, k, false);
}

void FontRenderer::renderString(const std::string &s, int_t i, int_t j, int_t k, bool flag)
{
	if (s.empty())
		return;
	if (flag)
	{
		int_t l = k & 0xff000000;
		k = (k & 0xfcfcfc) >> 2;
		k += l;
	}
	glBindTexture(GL_TEXTURE_2D, fontTextureName);
	float cr = (float)(k >> 16 & 0xff) / 255.0f;
	float cg = (float)(k >> 8 & 0xff) / 255.0f;
	float cb = (float)(k & 0xff) / 255.0f;
	float ca = (float)(k >> 24 & 0xff) / 255.0f;
	if (ca == 0.0f)
		ca = 1.0f;

	// Batch every glyph into ONE tessellated draw instead of one glCallLists per
	// character. On the iOS GLES shim each per-glyph display-list replay costs
	// several GL calls, which made text-heavy scenes (signs!) extremely slow.
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->setColorRGBA_F(cr, cg, cb, ca);

	const float x0 = (float)i;
	const float y0 = (float)j;
	const float f = 7.99f;
	float x = x0;

	for (size_t idx = 0; idx < s.length(); idx++)
	{
		if ((unsigned char)s[idx] == 0xa7 && idx + 1 < s.length())
		{
			char lower = (char)std::tolower((unsigned char)s[idx + 1]);
			int_t j1 = (int_t)std::string("0123456789abcdef").find(lower);
			if (j1 >= 0 && j1 <= 15)
			{
				int_t codeIdx = j1 + (flag ? 16 : 0);
				tess->setColorRGBA_F((float)colorCodeR[codeIdx] / 255.0f,
				                     (float)colorCodeG[codeIdx] / 255.0f,
				                     (float)colorCodeB[codeIdx] / 255.0f, ca);
				idx++;
				continue;
			}
		}

		int_t k1 = getCharIndex(s[idx]);
		if (k1 < 0)
			continue;
		int_t li = k1 + 32;
		int_t l1 = (li % 16) * 8;
		int_t k2 = (li / 16) * 8;
		float u0 = (float)l1 / 128.0f;
		float u1 = ((float)l1 + f) / 128.0f;
		float v0 = (float)k2 / 128.0f;
		float v1 = ((float)k2 + f) / 128.0f;
		tess->addVertexWithUV(x,     y0 + f, 0.0, u0, v1);
		tess->addVertexWithUV(x + f, y0 + f, 0.0, u1, v1);
		tess->addVertexWithUV(x + f, y0,     0.0, u1, v0);
		tess->addVertexWithUV(x,     y0,     0.0, u0, v0);
		x += (float)charWidth[li];
	}
	tess->draw();
}

int_t FontRenderer::getStringWidth(const std::string &s)
{
	if (s.empty())
		return 0;
	int_t i = 0;
	for (int_t j = 0; j < (int_t)s.length(); j++)
	{
		if ((unsigned char)s[j] == 0xa7)
		{
			j++;
			continue;
		}
		int_t k = getCharIndex(s[j]);
		if (k >= 0)
			i += charWidth[k + 32];
	}
	return i;
}

void FontRenderer::func_27278_a(const std::string &s, int_t i, int_t j, int_t k, int_t l)
{
	std::vector<std::string> lines = split(s, '\n');
	if (lines.size() > 1)
	{
		for (std::string &line : lines)
		{
			func_27278_a(line, i, j, k, l);
			j += func_27277_a(line, k);
		}
		return;
	}

	std::vector<std::string> words = split(s, ' ');
	int_t j1 = 0;
	while (j1 < (int_t)words.size())
	{
		std::string s1 = words[j1++] + " ";
		while (j1 < (int_t)words.size() && getStringWidth(s1 + words[j1]) < k)
			s1 += words[j1++] + " ";
		while (getStringWidth(s1) > k)
		{
			int_t k1 = 0;
			while (k1 + 1 < (int_t)s1.length() && getStringWidth(s1.substr(0, k1 + 1)) <= k)
				k1++;
			if (!s1.substr(0, k1).empty())
			{
				drawString(s1.substr(0, k1), i, j, l);
				j += 8;
			}
			s1 = s1.substr(k1);
		}
		if (!s1.empty() && s1.find_first_not_of(" \t\r\n") != std::string::npos)
		{
			drawString(s1, i, j, l);
			j += 8;
		}
	}
}

int_t FontRenderer::func_27277_a(const std::string &s, int_t i)
{
	std::vector<std::string> lines = split(s, '\n');
	if (lines.size() > 1)
	{
		int_t j = 0;
		for (std::string &line : lines)
			j += func_27277_a(line, i);
		return j;
	}

	std::vector<std::string> words = split(s, ' ');
	int_t l = 0;
	int_t i1 = 0;
	while (l < (int_t)words.size())
	{
		std::string s1 = words[l++] + " ";
		while (l < (int_t)words.size() && getStringWidth(s1 + words[l]) < i)
			s1 += words[l++] + " ";
		while (getStringWidth(s1) > i)
		{
			int_t j1 = 0;
			while (j1 + 1 < (int_t)s1.length() && getStringWidth(s1.substr(0, j1 + 1)) <= i)
				j1++;
			if (!s1.substr(0, j1).empty())
				i1 += 8;
			s1 = s1.substr(j1);
		}
		if (!s1.empty() && s1.find_first_not_of(" \t\r\n") != std::string::npos)
			i1 += 8;
	}
	if (i1 < 8)
		i1 += 8;
	return i1;
}

std::vector<std::string> FontRenderer::split(const std::string &s, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delimiter))
		result.push_back(item);
	if (!s.empty() && s.back() == delimiter)
		result.push_back("");
	return result;
}

int_t FontRenderer::getCharIndex(char c)
{
	std::size_t index = ChatAllowedCharacters::allowedCharacters.find(c);
	if (index != std::string::npos)
		return (int_t)index;
	unsigned char uc = (unsigned char)c;
	if (uc >= 32)
		return uc - 32;
	return -1;
}
