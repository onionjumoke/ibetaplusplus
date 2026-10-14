#include "NoiseGeneratorOctaves.h"
#include "NoiseGeneratorPerlin.h"

NoiseGeneratorOctaves::NoiseGeneratorOctaves(Random &random, int_t i)
    : biomeName(i)
{
    generatorCollection.resize(i);
    for (int_t j = 0; j < i; j++)
        generatorCollection[j] = new NoiseGeneratorPerlin(random);
}

double NoiseGeneratorOctaves::getExperiencePoints(double d, double d1)
{
    double d2 = 0.0;
    double d3 = 1.0;
    for (int_t i = 0; i < biomeName; i++)
    {
        d2 += generatorCollection[i]->getDropItemId(d * d3, d1 * d3) / d3;
        d3 /= 2.0;
    }
    return d2;
}

std::vector<double> &NoiseGeneratorOctaves::generateNoiseOctaves(std::vector<double> &ad,
    double d, double d1, double d2, int_t i, int_t j, int_t k,
    double d3, double d4, double d5)
{
    if (ad.empty())
        ad.assign(i * j * k, 0.0);
    else
        std::fill(ad.begin(), ad.end(), 0.0);

    double d6 = 1.0;
    for (int_t i1 = 0; i1 < biomeName; i1++)
    {
        generatorCollection[i1]->getCanSpawnHere(ad, d, d1, d2, i, j, k,
                                             d3 * d6, d4 * d6, d5 * d6, d6);
        d6 /= 2.0;
    }
    return ad;
}

std::vector<double> &NoiseGeneratorOctaves::getBiomeGenForCoords(std::vector<double> &ad,
    int_t i, int_t j, int_t k, int_t l, double d, double d1, double d2)
{
    return generateNoiseOctaves(ad, i, 10.0, j, k, 1, l, d, 1.0, d1);
}
