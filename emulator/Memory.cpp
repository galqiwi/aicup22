#include "Memory.h"
#include <cassert>

namespace Emulator {


void TMemory::Update(const TWorld &world) {
    if (LastUpdateTick >= world.CurrentTick) {
        return;
    }
    LastUpdateTick = world.CurrentTick;

    const auto constants = GetGlobalConstants();
    assert(constants);

    {
        std::vector<int> idsToErase;
        for (auto& [id, projectile]: ProjectileById) {
            if (world.ProjectileById.contains(id)) {
                continue;
            }
            projectile.Position = projectile.Position + projectile.Velocity / constants->realTicksPerSecond;
            projectile.LifeTime -= 1 / constants->realTicksPerSecond;
            if (projectile.LifeTime < 0) {
                idsToErase.push_back(id);
            }
        }
        for (auto& id: idsToErase) {
            ProjectileById.erase(id);
        }
        for (auto& [id, projectile]: world.ProjectileById) {
            ProjectileById[id] = projectile;
        }
    }

    if (world.CurrentTick % (int)(GetGlobalConstants()->realTicksPerSecond * 8) == 0) {
        LootById.clear();
    }
    if (world.CurrentTick % (int)(GetGlobalConstants()->realTicksPerSecond * 8) == GetGlobalConstants()->realTicksPerSecond * 4) {
        LootById2.clear();
    }

    for (const auto& [_, loot]: world.LootById) {
        LootById.insert({loot.Id, loot});
        LootById2.insert({loot.Id, loot});
    }

    for (auto& [_, unit]: world.UnitById) {
        if (unit.PlayerId != world.MyId) {
            continue;
        }
        auto fov = constants->fieldOfView;
        if (unit.Weapon) {
            fov -= unit.Aim * (constants->fieldOfView - constants->weapons[*unit.Weapon].aimFieldOfView);
        }

        std::vector<int> idsToErase;
        for (auto& [otherUnitId, otherUnit]: UnitById) {
            if (abs2(otherUnit.Position - unit.Position) > constants->viewDistance * constants->viewDistance) {
                continue;
            }
            if (norm(otherUnit.Position - unit.Position) * norm(unit.Direction) > cos(fov / 180 * M_PI / 2 / 2)) {
                idsToErase.push_back(otherUnitId);
            }
        }
        for (auto idToErase: idsToErase) {
            UnitById.erase(idToErase);
        }
    }

    for (const auto& [_, unit]: world.UnitById) {
        UnitById[unit.Id] = unit;
    }

    for (auto& [_, unit]: UnitById) {
        if (world.UnitById.contains(unit.Id)) {
            continue;
        }
        unit.Position = unit.Position + unit.Velocity / constants->realTicksPerSecond;
    }

    for (auto& [id, unit]: world.UnitById) {
        if (unit.PlayerId != world.MyId) {
            continue;
        }
        StateByUnitId.insert({id, TState{.UnitId = id}});
    }
}

void TMemory::InjectKnowledge(TWorld &world) {
    for (const auto& [id, projectile]: ProjectileById) {
        if (!world.ProjectileById.contains(id)) {
            world.ProjectileById[id] = projectile;
        }
    }

    for (const auto& [_, loot]: LootById) {
        world.LootById.insert({loot.Id, loot});
    }
    for (const auto& [_, loot]: LootById2) {
        world.LootById.insert({loot.Id, loot});
    }

    for (const auto& [_, unit]: UnitById) {
        if (!world.UnitById.contains(unit.Id)) {
            auto newUnit = unit;
            newUnit.Imaginable = true;
            world.UnitById.insert({unit.Id, newUnit});
        }
    }

    world.StateByUnitId = StateByUnitId;
}

void TMemory::UpdateSoundKnowledge(TWorld& world, const TSound &sound) {
    static auto constants = GetGlobalConstants();

    if (sound.TypeIndex > 3 /* hit */) {
        return;
    }

    auto soundProperties = constants->sounds[sound.TypeIndex];
    auto distToSound2 = abs2(world.UnitById.find(sound.UnitId)->second.Position - sound.Position);

    for (auto& [_, unit]: UnitById) {
        if (abs2(unit.Position - sound.Position) < std::max(distToSound2 * soundProperties.offset * soundProperties.offset * 2, 8.0)) {
            if (!world.UnitById.contains(unit.Id)) {
                unit.Position = sound.Position;
            }
            return;
        }
    }

    static int newUnitId = -1;
    --newUnitId;

    UnitById.insert(
        {newUnitId,
            TUnit{
                .Id = newUnitId,
                .PlayerId = -1,
                .Position = sound.Position,
                .Direction = Vector2D{1, 0},
                .Velocity = Vector2D{0, 0},
                .Health = constants->unitHealth,
                .Shield = constants->maxShield,
                .Aim = 1,
                .Weapon = 2,
            }
        });
}

void TMemory::ForgetLoot(int lootId) {
    LootById.erase(lootId);
    LootById2.erase(lootId);
}

void TMemory::RememberState(int unitId, const TState &state) {
    StateByUnitId[unitId] = state;
}

}