#include "LootPicker.h"
#include <cassert>

#include "World.h"
#include "Evaluation.h"

namespace Emulator {

bool LootIsAcceptable(const TWorld &world, const TUnit& unit, int lootId) {
    const auto& loot = world.LootById.find(lootId)->second;
    if (abs(loot.Position - world.Zone.currentCenter) > world.Zone.currentRadius - 1) {
        return false;
    }
    if (GetCombatSafety(world, unit, loot.Position) < 0) {
        return false;
    }
    return true;
}

std::optional<int> GetTargetLoot(const TWorld &world, int unitId) {
    if (world.LootIdByUnitId) {
        assert((*world.LootIdByUnitId).contains(unitId));
        return (*world.LootIdByUnitId).find(unitId)->second;
    }
    
    static auto constants = GetGlobalConstants();
    assert(constants);

    auto& unit = world.UnitById.find(unitId)->second;

    std::optional<double> minDist2 = std::nullopt;
    std::optional<int> output = std::nullopt;

    if (unit.ShieldPotions < constants->maxShieldPotionsInInventory) {
        for (auto& loot: world.LootByItemIndex.find(ShieldPotions)->second) {
            if (!LootIsAcceptable(world, unit, loot.Id)) {
                continue;
            }
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    if (unit.Weapon != 2) {
        for (auto& loot: world.LootByItemIndex.find(Weapon)->second) {
            if (!LootIsAcceptable(world, unit, loot.Id)) {
                continue;
            }
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    if (unit.Ammo[2] < constants->weapons[2].maxInventoryAmmo) {
        for (auto& loot: world.LootByItemIndex.find(Ammo)->second) {
            if (!LootIsAcceptable(world, unit, loot.Id)) {
                continue;
            }
            auto dist2 = abs2(loot.Position - unit.Position);
            if (!minDist2 || dist2 < minDist2) {
                minDist2 = dist2;
                output = loot.Id;
            }
        }
    }

    return output;
}

Vector2D GetTarget(int unitId, const TWorld &world, std::optional<int> loot) {
    if (loot) {
        return world.LootById.find(*loot)->second.Position;
    } else {
        auto angle = world.StateByUnitId.find(unitId)->second.spiralAngle;
        return world.Zone.nextCenter + Vector2D{cos(angle), sin(angle)} * (0.75 * world.Zone.nextRadius);
    }
}

Vector2D GetTarget(const TWorld &world, int unitId) {
    return GetTarget(unitId, world, GetTargetLoot(world, unitId));
}

}