#include "Evaluation.h"
#include "emulator/DebugSingleton.h"

#include <cassert>
#include <iostream>

namespace Emulator {

double EvaluateStrategy(const TStrategy &strategy, const TWorld &world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    auto& unit = currentWorld.UnitsById[unitId];

    double score = 0;

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();

        score += abs(unit.Position - world.Zone.nextCenter);
    }

    score -= unit.Health * 10000;

    return score;
}

}
