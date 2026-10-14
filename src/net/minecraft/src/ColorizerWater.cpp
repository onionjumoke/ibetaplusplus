#include "ColorizerWater.h"

std::vector<int_t> ColorizerWater::waterBuffer(0x10000);

void ColorizerWater::func_28182_a(const std::vector<int_t> &ai)
{
	waterBuffer = ai;
}
