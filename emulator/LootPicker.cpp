#include "LootPicker.h"
#include <cassert>

#include "World.h"

namespace Emulator {

std::optional<int> GetTargetLoot2(const TWorld &world, int unitId) {
    static auto constants = GetGlobalConstants();
    assert(constants);

    auto& unit = world.UnitById.find(unitId)->second;

    std::optional<double> minDist2 = std::nullopt;
    std::optional<int> output = std::nullopt;

    if (unit.ShieldPotions < constants->maxShieldPotionsInInventory) {
        auto lootId = world.LootIndexByItem.find(ShieldPotions)->second->ClosestPointId(unit.Position);
        if (lootId) {
            auto dist2 = abs2(world.LootById.find(*lootId)->second.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = *lootId;
            }
        }
    }

    if (unit.Weapon != 2) {
        auto lootId = world.LootIndexByItem.find(Weapon)->second->ClosestPointId(unit.Position);
        if (lootId) {
            auto dist2 = abs2(world.LootById.find(*lootId)->second.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = *lootId;
            }
        }
    }

    if (unit.Ammo[2] < constants->weapons[2].maxInventoryAmmo) {
        auto lootId = world.LootIndexByItem.find(Ammo)->second->ClosestPointId(unit.Position);
        if (lootId) {
            auto dist2 = abs2(world.LootById.find(*lootId)->second.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = *lootId;
            }
        }
    }

    return output;
}

std::optional<int> GetTargetLoot(const TWorld &world, int unitId) {
    static auto constants = GetGlobalConstants();
    assert(constants);

    auto& unit = world.UnitById.find(unitId)->second;

    std::optional<double> minDist2 = std::nullopt;
    std::optional<int> output = std::nullopt;

    if (unit.ShieldPotions < constants->maxShieldPotionsInInventory) {
        for (auto& loot: world.LootByItem.find(ShieldPotions)->second) {
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    if (unit.Weapon != 2) {
        for (auto& loot: world.LootByItem.find(Weapon)->second) {
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    if (unit.Ammo[2] < constants->weapons[2].maxInventoryAmmo) {
        for (auto& loot: world.LootByItem.find(Ammo)->second) {
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    assert(output == GetTargetLoot2(world, unitId));

    return output;
}

Vector2D GetTarget(const TWorld &world, std::optional<int> loot) {
    if (loot) {
        return world.LootById.find(*loot)->second.Position;
    } else {
        return world.Zone.nextCenter;
    }
}

Vector2D GetTarget(const TWorld &world, int unitId) {
    return GetTarget(world, GetTargetLoot(world, unitId));
}

}