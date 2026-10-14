#include "NoiseGeneratorPerlin.h"

NoiseGeneratorPerlin::NoiseGeneratorPerlin()
{
    Random r;
    *this = NoiseGeneratorPerlin(r);
}

NoiseGeneratorPerlin::NoiseGeneratorPerlin(Random &random)
    : xCoord(random.nextDouble() * 256.0),
      yCoord(random.nextDouble() * 256.0),
      zCoord(random.nextDouble() * 256.0)
{
    permutations.fill(0);
    for (int_t i = 0; i < 256; i++)
        permutations[i] = i;
    for (int_t j = 0; j < 256; j++)
    {
        int_t k = random.nextInt(256 - j) + j;
        int_t l = permutations[j];
        permutations[j] = permutations[k];
        permutations[k] = l;
        permutations[j + 256] = permutations[j];
    }
}

double NoiseGeneratorPerlin::lerp(double d, double d1, double d2)
{
    return d1 + d * (d2 - d1);
}

double NoiseGeneratorPerlin::getBlockMetadata(int_t i, double d, double d1)
{
    int_t j = i & 0xf;
    double d2 = (double)(1 - ((j & 8) >> 3)) * d;
    double d3 = j >= 4 ? (j != 12 && j != 14 ? d1 : d) : 0.0;
    return ((j & 1) != 0 ? -d2 : d2) + ((j & 2) != 0 ? -d3 : d3);
}

double NoiseGeneratorPerlin::grad(int_t i, double d, double d1, double d2)
{
    int_t j = i & 0xf;
    double d3 = j >= 8 ? d1 : d;
    double d4 = j >= 4 ? (j != 12 && j != 14 ? d2 : d) : d1;
    return ((j & 1) != 0 ? -d3 : d3) + ((j & 2) != 0 ? -d4 : d4);
}

double NoiseGeneratorPerlin::getDropItemId(double d, double d1)
{
    return generateNoise(d, d1, 0.0);
}

double NoiseGeneratorPerlin::generateNoise(double d, double d1, double d2)
{
    double d3 = d + xCoord;
    double d4 = d1 + yCoord;
    double d5 = d2 + zCoord;
    int_t i = (int_t)d3; if (d3 < (double)i) i--;
    int_t j = (int_t)d4; if (d4 < (double)j) j--;
    int_t k = (int_t)d5; if (d5 < (double)k) k--;
    int_t l = i & 0xff, i1 = j & 0xff, j1 = k & 0xff;
    d3 -= i; d4 -= j; d5 -= k;
    double d6 = d3*d3*d3*(d3*(d3*6.0-15.0)+10.0);
    double d7 = d4*d4*d4*(d4*(d4*6.0-15.0)+10.0);
    double d8 = d5*d5*d5*(d5*(d5*6.0-15.0)+10.0);
    int_t k1 = permutations[l] + i1;
    int_t l1 = permutations[k1] + j1;
    int_t i2 = permutations[k1+1] + j1;
    int_t j2 = permutations[l+1] + i1;
    int_t k2 = permutations[j2] + j1;
    int_t l2 = permutations[j2+1] + j1;
    return lerp(d8,
        lerp(d7,
            lerp(d6, grad(permutations[l1],d3,d4,d5), grad(permutations[k2],d3-1.0,d4,d5)),
            lerp(d6, grad(permutations[i2],d3,d4-1.0,d5), grad(permutations[l2],d3-1.0,d4-1.0,d5))),
        lerp(d7,
            lerp(d6, grad(permutations[l1+1],d3,d4,d5-1.0), grad(permutations[k2+1],d3-1.0,d4,d5-1.0)),
            lerp(d6, grad(permutations[i2+1],d3,d4-1.0,d5-1.0), grad(permutations[l2+1],d3-1.0,d4-1.0,d5-1.0))));
}

void NoiseGeneratorPerlin::getCanSpawnHere(std::vector<double> &ad, double d, double d1, double d2,
                                      int_t i, int_t j, int_t k,
                                      double d3, double d4, double d5, double d6)
{
    if (j == 1)
    {
        int_t j3 = 0;
        double d12 = 1.0 / d6;
        for (int_t i4 = 0; i4 < i; i4++)
        {
            double d14 = (d + i4) * d3 + xCoord;
            int_t j4 = (int_t)d14; if (d14 < (double)j4) j4--;
            int_t k4 = j4 & 0xff;
            d14 -= j4;
            double d17 = d14*d14*d14*(d14*(d14*6.0-15.0)+10.0);
            for (int_t l4 = 0; l4 < k; l4++)
            {
                double d19 = (d2 + l4) * d5 + zCoord;
                int_t j5 = (int_t)d19; if (d19 < (double)j5) j5--;
                int_t l5 = j5 & 0xff;
                d19 -= j5;
                double d21 = d19*d19*d19*(d19*(d19*6.0-15.0)+10.0);
                int_t l  = permutations[k4] + 0;
                int_t j1 = permutations[l] + l5;
                int_t k1 = permutations[k4+1] + 0;
                int_t l1 = permutations[k1] + l5;
                double d9  = lerp(d17, getBlockMetadata(permutations[j1],d14,d19), grad(permutations[l1],d14-1.0,0.0,d19));
                double d11 = lerp(d17, grad(permutations[j1+1],d14,0.0,d19-1.0), grad(permutations[l1+1],d14-1.0,0.0,d19-1.0));
                ad[j3++] += lerp(d21, d9, d11) * d12;
            }
        }
        return;
    }
    int_t i1 = 0;
    double d7 = 1.0 / d6;
    int_t i2 = -1;
    double d13=0, d15=0, d16=0, d18=0;
    for (int_t i5 = 0; i5 < i; i5++)
    {
        double d20 = (d + i5) * d3 + xCoord;
        int_t k5 = (int_t)d20; if (d20 < (double)k5) k5--;
        int_t i6 = k5 & 0xff;
        d20 -= k5;
        double d22 = d20*d20*d20*(d20*(d20*6.0-15.0)+10.0);
        for (int_t j6 = 0; j6 < k; j6++)
        {
            double d24 = (d2 + j6) * d5 + zCoord;
            int_t k6 = (int_t)d24; if (d24 < (double)k6) k6--;
            int_t l6 = k6 & 0xff;
            d24 -= k6;
            double d25 = d24*d24*d24*(d24*(d24*6.0-15.0)+10.0);
            for (int_t i7 = 0; i7 < j; i7++)
            {
                double d26 = (d1 + i7) * d4 + yCoord;
                int_t j7 = (int_t)d26; if (d26 < (double)j7) j7--;
                int_t k7 = j7 & 0xff;
                d26 -= j7;
                double d27 = d26*d26*d26*(d26*(d26*6.0-15.0)+10.0);
                if (i7 == 0 || k7 != i2)
                {
                    i2 = k7;
                    int_t j2 = permutations[i6]+k7, k2=permutations[j2]+l6, l2=permutations[j2+1]+l6;
                    int_t i3 = permutations[i6+1]+k7, k3=permutations[i3]+l6, l3=permutations[i3+1]+l6;
                    d13 = lerp(d22, grad(permutations[k2],d20,d26,d24), grad(permutations[k3],d20-1.0,d26,d24));
                    d15 = lerp(d22, grad(permutations[l2],d20,d26-1.0,d24), grad(permutations[l3],d20-1.0,d26-1.0,d24));
                    d16 = lerp(d22, grad(permutations[k2+1],d20,d26,d24-1.0), grad(permutations[k3+1],d20-1.0,d26,d24-1.0));
                    d18 = lerp(d22, grad(permutations[l2+1],d20,d26-1.0,d24-1.0), grad(permutations[l3+1],d20-1.0,d26-1.0,d24-1.0));
                }
                ad[i1++] += lerp(d25, lerp(d27,d13,d15), lerp(d27,d16,d18)) * d7;
            }
        }
    }
}
