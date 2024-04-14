#pragma once

#include "core/types.h"

template<class ItemType>
struct ItemStack
{
    ItemType item{};
    u32 count{};
};