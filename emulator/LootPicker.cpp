#include "LootPicker.h"

#include "World.h"

namespace Emulator {

std::optional<int> GetTargetLoot(const TWorld &world, int unitId) {
    auto& unit = world.UnitsById.find(unitId)->second;

    std::optional<double> minDist2 = std::nullopt;
    std::optional<int> output = std::nullopt;
    for (auto& [_, loot]: world.LootById) {
        if (loot.Item == Weapon) {
            if (loot.WeaponType != 2 /* bow */) {
                continue;
            }
            if (unit.Weapon == 2) {
                continue;
            }
        }

        if (loot.Item == Ammo) {
            if (loot.WeaponType != 2 /* bow */) {
                continue;
            }
            if (unit.Ammo[2] == GetGlobalConstants()->weapons[2].maxInventoryAmmo) {
                continue;
            }
        }

        if (loot.Item == ShieldPotions) {
            if (unit.ShieldPotions == GetGlobalConstants()->maxShieldPotionsInInventory) {
                continue;
            }
        }

        if (abs(loot.Position - world.Zone.currentCenter) > world.Zone.currentRadius - 30) {
            continue;
        }

        auto dist2 = abs(loot.Position - unit.Position);
        if (!minDist2 || dist2 < minDist2) {
            minDist2 = dist2;
            output = loot.Id;
        }
    }

    return output;
}

Vector2D GetTarget(const TWorld &world, int unitId) {
    auto lootId = GetTargetLoot(world, unitId);
    if (lootId) {
        return world.LootById.find(*lootId)->second.Position;
    } else {
        return world.Zone.nextCenter;
    }
}

}