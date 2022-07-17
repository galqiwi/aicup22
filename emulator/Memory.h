#pragma once

#include "public.h"

#include "Sound.h"
#include "World.h"

#include <unordered_map>

namespace Emulator {

struct TMemory {
public:
    void Update(const TWorld& world);
    void UpdateSoundKnowledge(TWorld& world, const TSound& sound);
    void RememberState(int unitId, const TState& state);
    void InjectKnowledge(TWorld& world);
    void ForgetLoot(int lootId);
private:
    std::unordered_map<int, TLoot> LootById;
    std::unordered_map<int, TLoot> LootById2;
    std::unordered_map<int, TUnit> UnitById;
    std::unordered_map<int, TProjectile> ProjectileById;
    std::unordered_map<int, TState> StateByUnitId;
    int LastUpdateTick{-1};
};

}