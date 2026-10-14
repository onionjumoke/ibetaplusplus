#include "NoiseGeneratorOctaves2.h"
#include "NoiseGenerator2.h"
#include <algorithm>

NoiseGeneratorOctaves2::NoiseGeneratorOctaves2(Random &random, int_t i)
    : dialTexture(i)
{
    needleTexture.resize(i);
    for (int_t j = 0; j < i; j++)
        needleTexture[j] = new NoiseGenerator2(random);
}

std::vector<double> &NoiseGeneratorOctaves2::getBlockMaterial(std::vector<double> &ad,
    double d, double d1, int_t i, int_t j, double d2, double d3, double d4)
{
    return setBlockMetadata(ad, d, d1, i, j, d2, d3, d4, 0.5);
}

std::vector<double> &NoiseGeneratorOctaves2::setBlockMetadata(std::vector<double> &ad,
    double d, double d1, int_t i, int_t j, double d2, double d3, double d4, double d5)
{
    d2 /= 1.5;
    d3 /= 1.5;
    if (ad.empty() || (int_t)ad.size() < i * j)
        ad.assign(i * j, 0.0);
    else
        std::fill(ad.begin(), ad.end(), 0.0);

    double d6 = 1.0;
    double d7 = 1.0;
    for (int_t l = 0; l < dialTexture; l++)
    {
        needleTexture[l]->generatePerlinNoise(ad, d, d1, i, j, d2 * d7, d3 * d7, 0.55 / d6);
        d7 *= d4;
        d6 *= d5;
    }
    return ad;
}
