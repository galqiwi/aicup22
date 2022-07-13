#include "Memory.h"

namespace Emulator {


void TMemory::Update(const TWorld &world) {
    if (world.CurrentTick % (int)(GetGlobalConstants()->ticksPerSecond * 2) == 0) {
        LootById.clear();
        return;
    }
    for (const auto& [_, loot]: world.LootById) {
        LootById.insert({loot.Id, loot});
    }
}

void TMemory::InjectKnowledge(TWorld &world) {
    for (const auto& [_, loot]: LootById) {
        world.LootById.insert({loot.Id, loot});
    }
}

void TMemory::ForgetLoot(int lootId) {
    LootById.erase(lootId);
}

}