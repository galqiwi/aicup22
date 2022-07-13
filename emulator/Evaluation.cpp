#include "Evaluation.h"
#include "emulator/DebugSingleton.h"

#include <cassert>
#include <iostream>

namespace Emulator {

double EvaluateWorld(const TWorld& world, const TUnit& unit) {
    auto score = - unit.Health * 10000;

    // TODO: support more than one player
    if (world.UnitsById.size() > 1) {
        for (auto& [otherUnitId, otherUnit]: world.UnitsById) {
            if (otherUnitId == unit.Id) {
                continue;
            }
            score += fabs(abs(unit.Position - otherUnit.Position) - 30);
        }
    } else {
        score += abs(unit.Position - world.Zone.nextCenter);
    }

    return score;
}

double EvaluateStrategy(const TStrategy &strategy, const TWorld& world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    const auto& unit = currentWorld.UnitsById[unitId];

    double score = 0;

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();

        score += EvaluateWorld(world, unit);
    }

    return score;
}

}
