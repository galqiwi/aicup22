#pragma once

#include "World.h"
#include "public.h"

#include <memory>
#include <unordered_map>

namespace Emulator {

struct IClosestPointIndex {
    virtual std::optional<int> ClosestPointId(Vector2D point) = 0;
};

IClosestPointIndexPtr CreateClosestLootIndex(const std::unordered_map<ELootItem, std::vector<TLoot>>& loot, ELootItem item);

}