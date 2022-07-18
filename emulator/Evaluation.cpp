#include "Evaluation.h"
#include "emulator/DebugSingleton.h"

#include "LootPicker.h"

#include <cassert>
#include <iostream>

namespace Emulator {

bool operator<(TOptionalDouble a, TOptionalDouble b) {
    if (a.value == b.value) {
        return false;
    }

    return a.value.value_or(0) < b.value.value_or(0);
}

TOptionalDouble operator+(TOptionalDouble a, TOptionalDouble b) {
    if (!a.value && !b.value) {
        return {std::nullopt};
    }
    return {a.value.value_or(0) + b.value.value_or(0)};
}

TScore operator+(TScore a, TScore b) {
    return {
        get<0>(a) + get<0>(b),
        get<1>(a) + get<1>(b),
        get<2>(a) + get<2>(b),
    };
}

double AmmoCoefficient(int ammo) {
    int enoughToKill = 10;
    if (ammo >= enoughToKill) {
        return 1;
    }
    return ((double) ammo) / ((double)enoughToKill);
}


double GetCombatSafety(const TWorld& world, const TUnit& unit, Vector2D unitPosition) {
    double combatSafety = 0;
    std::optional<double> minDist = std::nullopt;
    for (auto& [_, otherUnit]: world.UnitById) {
        if (otherUnit.PlayerId == world.MyId) {
            continue;
        }

        auto dist = abs(unitPosition - otherUnit.Position);
        if (dist < otherUnit.GetCombatRadius()) {
            auto distanceCoefficient = (otherUnit.GetCombatRadius() - dist) / otherUnit.GetCombatRadius();
            combatSafety -= (otherUnit.Health + otherUnit.Shield + 1) * distanceCoefficient * distanceCoefficient;
        }
        if (!minDist || dist < *minDist) {
            minDist = dist;
        }
    }

    if (minDist && *minDist < unit.GetCombatRadius() && unit.Weapon == 2 && unit.Shield > 0 && unit.Ammo[2] > 0) {
        auto distanceCoefficient = (unit.GetCombatRadius() - *minDist) / unit.GetCombatRadius();
        combatSafety += (unit.Health + unit.Shield) * distanceCoefficient * distanceCoefficient * AmmoCoefficient(unit.Ammo[2]);
    }

    return combatSafety;
}

double GetCombatSafety(const TWorld& world, const TUnit& unit) {
    return GetCombatSafety(world, unit, unit.Position);
}

TScore EvaluateWorld(const TWorld& world, const TUnit& unit) {
    static auto constants = GetGlobalConstants();

    TScore score = {0, {std::nullopt}, 0};
    std::get<0>(score) = constants->unitHealth - unit.Health;

    auto combatSafety = GetCombatSafety(world, unit);

    get<1>(score).value = -combatSafety;

    if (combatSafety >= 0 && !(unit.Weapon == 2 && unit.Shield > 0 && unit.Ammo[2] > 0)) {
        get<1>(score).value = std::nullopt;
    }

    auto distScore = abs(unit.Position - GetTarget(world, unit.Id));
    if (distScore < GetGlobalConstants()->unitRadius / 2) {
        distScore -= 10;
    }

    std::get<2>(score) = distScore;

    return score;
}

TScore EvaluateStrategy(const TStrategy &strategy, const TWorld& world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    const auto& unit = currentWorld.UnitById[unitId];

    TScore score = {0, {std::nullopt}, 0};

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.PrepareEmulation();
        auto order = strategy.GetOrder(currentWorld, unitId);
        currentWorld.EmulateOrder(order);
        currentWorld.StateByUnitId[unitId].Update(currentWorld, order);
        currentWorld.Tick();

        score = score + EvaluateWorld(world, unit);
    }

    return score;
}

}
