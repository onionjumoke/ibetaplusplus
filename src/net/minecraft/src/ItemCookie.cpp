#include "ItemCookie.h"

ItemCookie::ItemCookie(int i, int j, bool flag, int k)
    : ItemFood(i, j, flag) {
    maxStackSize = k;
}
