#include "Evaluation.h"
#include "emulator/DebugSingleton.h"

#include <cassert>

namespace Emulator {

double EvaluateStrategy(const TStrategy &strategy, const TWorld &world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    auto& unit = currentWorld.UnitsById[unitId];

    std::vector<model::Vec2> line;
    line.reserve(untilTick - currentWorld.CurrentTick);

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();
        line.push_back(unit.Position.ToApi());
    }
    assert(GetGlobalDebugInterface());
//    GetGlobalDebugInterface()->addPolyLine(std::move(line), 0.15, debugging::Color(1, 0, 0, 1));

    return -abs2(unit.Position);
}

}
