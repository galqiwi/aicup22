#pragma once

#include "public.h"

#include "World.h"

#include <unordered_map>

namespace Emulator {

struct TMemory {
public:
    void Update(const TWorld& world);
    void InjectKnowledge(TWorld& world);
    void ForgetLoot(int lootId);
private:
    std::unordered_map<int, TLoot> LootById;
};

}