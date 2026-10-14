#include "NoiseGenerator2.h"
#include <cmath>

const int_t NoiseGenerator2::amplitude[12][3] = {
    {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
    {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
    {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};
const double NoiseGenerator2::persistence = 0.5 * (std::sqrt(3.0) - 1.0);
const double NoiseGenerator2::octaves = (3.0 - std::sqrt(3.0)) / 6.0;

NoiseGenerator2::NoiseGenerator2()
{
    Random r;
    *this = NoiseGenerator2(r);
}

NoiseGenerator2::NoiseGenerator2(Random &random)
    : seed(random.nextDouble() * 256.0),
      permutations(random.nextDouble() * 256.0),
      lacunarity(random.nextDouble() * 256.0)
{
    frequency.fill(0);
    for (int_t i = 0; i < 256; i++) frequency[i] = i;
    for (int_t j = 0; j < 256; j++)
    {
        int_t k = random.nextInt(256 - j) + j;
        int_t l = frequency[j];
        frequency[j] = frequency[k];
        frequency[k] = l;
        frequency[j + 256] = frequency[j];
    }
}

int_t NoiseGenerator2::wrap(double d)
{
    return d <= 0.0 ? (int_t)d - 1 : (int_t)d;
}

double NoiseGenerator2::generateNoiseOctaves(const int_t ai[2], double d, double d1)
{
    return (double)ai[0] * d + (double)ai[1] * d1;
}

void NoiseGenerator2::generatePerlinNoise(std::vector<double> &ad, double d, double d1,
                                   int_t i, int_t j, double d2, double d3, double d4)
{
    int_t k = 0;
    for (int_t l = 0; l < i; l++)
    {
        double d5 = (d + l) * d2 + seed;
        for (int_t i1 = 0; i1 < j; i1++)
        {
            double d6 = (d1 + i1) * d3 + permutations;
            double d10 = (d5 + d6) * persistence;
            int_t j1 = wrap(d5 + d10);
            int_t k1 = wrap(d6 + d10);
            double d11 = (double)(j1 + k1) * octaves;
            double d12 = d5 - ((double)j1 - d11);
            double d13 = d6 - ((double)k1 - d11);
            int_t l1, i2;
            if (d12 > d13) { l1=1; i2=0; } else { l1=0; i2=1; }
            double d16 = (d12 - l1) + octaves;
            double d17 = (d13 - i2) + octaves;
            double d18 = (d12 - 1.0) + 2.0*octaves;
            double d19 = (d13 - 1.0) + 2.0*octaves;
            int_t j2 = j1 & 0xff, k2 = k1 & 0xff;
            int_t l2 = frequency[j2 + frequency[k2]] % 12;
            int_t i3 = frequency[j2+l1 + frequency[k2+i2]] % 12;
            int_t j3 = frequency[j2+1 + frequency[k2+1]] % 12;
            double d20 = 0.5 - d12*d12 - d13*d13;
            double d7 = (d20 < 0.0) ? 0.0 : (d20*=d20, d20*d20*generateNoiseOctaves(amplitude[l2], d12, d13));
            double d21 = 0.5 - d16*d16 - d17*d17;
            double d8 = (d21 < 0.0) ? 0.0 : (d21*=d21, d21*d21*generateNoiseOctaves(amplitude[i3], d16, d17));
            double d22 = 0.5 - d18*d18 - d19*d19;
            double d9 = (d22 < 0.0) ? 0.0 : (d22*=d22, d22*d22*generateNoiseOctaves(amplitude[j3], d18, d19));
            ad[k++] += 70.0 * (d7 + d8 + d9) * d4;
        }
    }
}
