#include "Evaluation.h"
#include "emulator/DebugSingleton.h"

#include "LootPicker.h"

#include <cassert>
#include <iostream>

namespace Emulator {

bool operator<(TCombatSafetyScore a, TCombatSafetyScore b) {
    if (a.value == b.value) {
        return false;
    }

    if (!a.value) {
        a.value = 0;
    }
    if (!b.value) {
        b.value = 0;
    }
    return a.value < b.value;
}

TCombatSafetyScore operator+(TCombatSafetyScore a, TCombatSafetyScore b) {
    if (!a.value && !b.value) {
        return {std::nullopt};
    }
    if (!a.value) {
        a.value = 0;
    }
    if (!b.value) {
        b.value = 0;
    }
    return {*a.value + *b.value};
}

TScore operator+(TScore a, TScore b) {
    return {
        get<0>(a) + get<0>(b),
        get<1>(a) + get<1>(b),
        get<2>(a) + get<2>(b),
    };
}

TScore EvaluateWorld(const TWorld& world, const TUnit& unit) {
    static auto constants = GetGlobalConstants();

    TScore score = {0, {std::nullopt}, 0};
    std::get<0>(score) = constants->unitHealth - unit.Health;

    // TODO: support more than one player
    if (world.UnitsById.size() > 1) {
        double combatSafety = 0;
        double combatRadius = 40;
        bool canFight = false;
        for (auto& [_, otherUnit]: world.UnitsById) {
            if (otherUnit.PlayerId == unit.PlayerId) {
                continue;
            }
            if (abs2(unit.Position - otherUnit.Position) < combatRadius * combatRadius) {
                canFight = true;
                combatSafety -= otherUnit.Health + otherUnit.Shield;
            }
        }
        if (canFight) {
            combatSafety += unit.Health + unit.Shield;
        }
        get<1>(score).value = -combatSafety;
    }

//    return score;

    auto distScore = abs(unit.Position - GetTarget(world, unit.Id));
    if (distScore < GetGlobalConstants()->unitRadius / 2) {
        distScore -= 10;
    }
    std::get<2>(score) = distScore;

    return score;
}

TScore EvaluateStrategy(const TStrategy &strategy, const TWorld& world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    const auto& unit = currentWorld.UnitsById[unitId];

    TScore score = {0, {std::nullopt}, 0};

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.PrepareEmulation();
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();

        score = score + EvaluateWorld(world, unit);
    }

    return score;
}

}
