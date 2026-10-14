#include "ItemArmor.h"

const int ItemArmor::damageReduceAmountArray[4] = { 3, 8, 6, 3 };
const int ItemArmor::maxDamageArray[4] = { 11, 16, 15, 13 };

ItemArmor::ItemArmor(int i, int j, int k, int l)
    : Item(i), armorLevel(j), armorType(l), renderIndex(k),
      damageReduceAmount(damageReduceAmountArray[l]) {
    setMaxDamage(maxDamageArray[l] * 3 << j);
    maxStackSize = 1;
}
