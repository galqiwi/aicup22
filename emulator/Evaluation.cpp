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
        .HealthScore = a.HealthScore + b.HealthScore,
        .CombatSafetyScore = a.CombatSafetyScore + b.CombatSafetyScore,
        .TargetDistanceScore = a.TargetDistanceScore + b.TargetDistanceScore,
    };
}

bool operator<(TScore a, TScore b) {
    assert(a.Mode == b.Mode);
    if (a.HealthScore < b.HealthScore) {
        return true;
    }
    if (b.HealthScore < a.HealthScore) {
        return false;
    }
    if (a.CombatSafetyScore < b.CombatSafetyScore) {
        return true;
    }
    if (b.CombatSafetyScore < a.CombatSafetyScore) {
        return false;
    }
    if (a.TargetDistanceScore < b.TargetDistanceScore) {
        return true;
    }
    if (b.TargetDistanceScore < a.TargetDistanceScore) {
        return false;
    }
    return false;
}

double AmmoCoefficient(int ammo) {
    int enoughToKill = 5;
    if (ammo >= enoughToKill) {
        return 1;
    }
    return ((double) ammo) / ((double)enoughToKill);
}

double GetPower(const TUnit& killer, const TUnit& victim) {
    static auto constants = GetGlobalConstants();
    assert(constants);

    if (!killer.Weapon) {
        return 0;
    }

    auto& weapon = constants->weapons[*killer.Weapon];

    long hitsToKill = 0;
    if (victim.Shield > 0.01) {
        hitsToKill += std::lround(std::ceil(victim.Shield / weapon.projectileDamage));
    }
    if (victim.Health > 0.01) {
        hitsToKill += std::lround(std::ceil(victim.Health / weapon.projectileDamage));
    }

    hitsToKill = std::max(hitsToKill, 1l);

    return weapon.roundsPerSecond / ((double)hitsToKill);
}

double GetCombatSafety(const TWorld& world, const TUnit& unit, Vector2D unitPosition) {
    const auto& state = world.StateByUnitId.find(unit.Id)->second;
    double combatSafety = 0;
    std::optional<double> minDist = std::nullopt;
    int otherUnitId = -1;

    double radiusCoefficient = state.AutomatonState == RES_GATHERING ? 0.3:1;

    for (auto& [_, otherUnit]: world.UnitById) {
        if (otherUnit.PlayerId == world.MyId) {
            continue;
        }

        auto dist = abs(unitPosition - otherUnit.Position);
        auto otherUnitCombatRadius = otherUnit.GetCombatRadius() * radiusCoefficient;
        if (dist < otherUnitCombatRadius) {
            auto distanceCoefficient = (otherUnitCombatRadius - dist) / otherUnitCombatRadius;
            combatSafety -= (GetPower(otherUnit, unit) + 1e-4) * distanceCoefficient * distanceCoefficient;
        }
        if (!minDist || dist < *minDist) {
            minDist = dist;
            otherUnitId = otherUnit.Id;
        }
    }

    auto unitCombatRadius = unit.GetCombatRadius() * radiusCoefficient;
    if (minDist && *minDist < unitCombatRadius && state.AutomatonState != RES_GATHERING) {
        const auto& otherUnit = world.UnitById.find(otherUnitId)->second;
        auto distanceCoefficient = (unitCombatRadius - *minDist) / unitCombatRadius;
        combatSafety += GetPower(unit, otherUnit) * distanceCoefficient * distanceCoefficient;
    }

    return combatSafety;
}

double GetCombatSafety(const TWorld& world, const TUnit& unit) {
    return GetCombatSafety(world, unit, unit.Position);
}

TScore EvaluateResGatheringWorld(const TWorld& world, const TUnit& unit) {
    const auto& state = world.StateByUnitId.find(unit.Id)->second;
    static auto constants = GetGlobalConstants();
    TScore score = {0, {std::nullopt}, 0};
    score.HealthScore = constants->unitHealth - unit.Health;
    score.CombatSafetyScore.value = std::nullopt;

    auto distScore = abs(unit.Position - GetTarget(world, unit.Id, true));
    score.TargetDistanceScore = distScore;

    return score;
}

TScore EvaluateWorld(const TWorld& world, const TUnit& unit) {
    const auto& state = world.StateByUnitId.find(unit.Id)->second;

    if (state.AutomatonState == RES_GATHERING) {
        return EvaluateResGatheringWorld(world, unit);
    }

    static auto constants = GetGlobalConstants();

    TScore score = {0, {std::nullopt}, 0};

    score.HealthScore = constants->unitHealth - unit.Health;

    auto combatSafety = GetCombatSafety(world, unit);

    score.CombatSafetyScore.value = -combatSafety;

    auto distScore = abs(unit.Position - GetTarget(world, unit.Id, true));

    score.TargetDistanceScore = distScore;

    return score;
}

TScore EvaluateStrategy(const TStrategy &strategy, const TWorld& world, int unitId, int untilTick) {
    assert(world.StateByUnitId.contains(unitId));
    const auto& state = world.StateByUnitId.find(unitId)->second;
    TWorld currentWorld = world;
    const auto& unit = currentWorld.UnitById[unitId];

    TScore score = {0, {std::nullopt}, 0};

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.StateByUnitId[unitId].Sync(currentWorld);
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
